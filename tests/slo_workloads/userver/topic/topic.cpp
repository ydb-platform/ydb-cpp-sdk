#include <tests/slo_workloads/topic/topic.h>

#include <tests/slo_workloads/userver/key_value/userver_table_client.h>

#include <userver/engine/async.hpp>
#include <userver/engine/future_status.hpp>
#include <userver/engine/get_all.hpp>
#include <userver/engine/sleep.hpp>
#include <userver/engine/task/task_with_result.hpp>
#include <userver/engine/wait_all_checked.hpp>
#include <userver/ydb/topic.hpp>

#include <util/string/builder.h>

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <stop_token>
#include <utility>
#include <vector>

using namespace NYdb::NTopic;

namespace {

using namespace std::chrono_literals;

class TTopicWriters {
public:
  TTopicWriters(userver::ydb::TopicClient &client, TTopicRunContext &context)
      : Client_(client), Context_(context) {}

  userver::engine::TaskWithResult<void> Run(std::stop_token stopToken) {
    return userver::engine::AsyncNoTracing([this, stopToken] {
      std::vector<userver::engine::TaskWithResult<void>> writers;
      writers.reserve(Context_.GetOptions().WriterCount);
      for (std::uint32_t i = 0; i < Context_.GetOptions().WriterCount; ++i) {
        writers.push_back(userver::engine::AsyncNoTracing(
            [this, i, stopToken] { RunWriter(i, stopToken); }));
      }

      try {
        userver::engine::GetAll(writers);
      } catch (const std::exception &e) {
        if (!stopToken.stop_requested()) {
          Context_.Fail(TStringBuilder()
                        << "topic writer group failed: " << e.what());
        }
      }
    });
  }

private:
  void RunWriter(std::uint32_t writerIndex, std::stop_token stopToken) {
    std::optional<userver::ydb::TopicWriteSession> session;
    std::shared_ptr<TStatUnit> writeStat;
    std::optional<TContinuationToken> continuationToken;
    bool ownFailure = false;

    try {
      session.emplace(Client_.CreateWriteSession(
          MakeTopicWriteSettings(Context_.GetOptions(), writerIndex)));

      while (!stopToken.stop_requested()) {
        if (continuationToken && !writeStat) {
          writeStat = Context_.StartWrite();
          session->Write(std::move(*continuationToken),
                         TWriteMessage("message"));
          continuationToken.reset();
          continue;
        }

        auto event = session->GetEvent();

        bool acked = false;
        if (!HandleTopicWriteEvent(event, continuationToken, acked, Context_)) {
          ownFailure = true;
          break;
        }
        if (acked && writeStat) {
          Context_.FinishWrite(writeStat, true);
          writeStat.reset();
        }
      }
    } catch (const std::exception &e) {
      if (!stopToken.stop_requested()) {
        ownFailure = true;
        Context_.Fail(TStringBuilder() << "topic writer " << writerIndex
                                       << " failed: " << e.what());
      }
    }

    if (writeStat) {
      if (ownFailure) {
        Context_.FinishWrite(writeStat, false);
      } else {
        Context_.CancelWrite(writeStat);
      }
    }
    if (session) {
      try {
        session->Close(0ms);
      } catch (const std::exception &) {
      }
    }
  }

  userver::ydb::TopicClient &Client_;
  TTopicRunContext &Context_;
};

class TTopicReaders {
public:
  TTopicReaders(userver::ydb::TopicClient &client, TTopicRunContext &context)
      : Client_(client), Context_(context) {}

  userver::engine::TaskWithResult<void> Run(std::stop_token stopToken) {
    return userver::engine::AsyncNoTracing([this, stopToken] {
      std::vector<userver::engine::TaskWithResult<void>> readers;
      readers.reserve(Context_.GetOptions().ReaderCount);
      for (std::uint32_t i = 0; i < Context_.GetOptions().ReaderCount; ++i) {
        readers.push_back(userver::engine::AsyncNoTracing(
            [this, i, stopToken] { RunReader(i, stopToken); }));
      }

      try {
        userver::engine::GetAll(readers);
      } catch (const std::exception &e) {
        if (!stopToken.stop_requested()) {
          Context_.Fail(TStringBuilder()
                        << "topic reader group failed: " << e.what());
        }
      }
    });
  }

private:
  void RunReader(std::uint32_t readerIndex, std::stop_token stopToken) {
    std::optional<userver::ydb::TopicReadSession> session;

    try {
      session.emplace(Client_.CreateReadSession(
          MakeTopicReadSettings(Context_.GetOptions())));
      while (!stopToken.stop_requested()) {
        auto events = session->GetEvents();

        for (auto &event : events) {
          if (!HandleTopicReadEvent(event, Context_)) {
            break;
          }
        }
      }
    } catch (const std::exception &e) {
      if (!stopToken.stop_requested()) {
        Context_.Fail(TStringBuilder() << "topic reader " << readerIndex
                                       << " failed: " << e.what());
      }
    }

    if (session) {
      try {
        session->Close(0ms);
      } catch (const std::exception &) {
      }
    }
  }

  userver::ydb::TopicClient &Client_;
  TTopicRunContext &Context_;
};

} // namespace

int DoRun(TDatabaseOptions &dbOptions, int argc, char **argv) {
  TTopicOptions options(dbOptions);
  if (!ParseTopicOptions(argc, argv, options)) {
    return EXIT_FAILURE;
  }

  std::stop_source stopSource;
  auto &client = userver_slo::GetTopicClient();
  TTopicRunContext context(options, stopSource);
  TTopicReaders readers(client, context);
  TTopicWriters writers(client, context);

  context.Start();
  auto stopTask =
      userver::engine::AsyncNoTracing([stopSource, &options]() mutable {
        userver::engine::InterruptibleSleepFor(
            std::chrono::seconds(options.SecondsToRun));
        stopSource.request_stop();
      });
  auto readerTask = readers.Run(stopSource.get_token());
  auto writerTask = writers.Run(stopSource.get_token());

  try {
    const auto status = userver::engine::WaitAllCheckedFor(
        std::chrono::seconds(options.SecondsToRun) + 60s, readerTask,
        writerTask);
    if (status != userver::engine::FutureStatus::kReady) {
      context.Fail("topic workers did not stop within 60 seconds");
      readerTask.RequestCancel();
      writerTask.RequestCancel();
    }
    userver::engine::GetAll(readerTask, writerTask);
  } catch (const std::exception &e) {
    if (!context.Failed()) {
      context.Fail(TStringBuilder() << "topic workload failed: " << e.what());
    }
    if (readerTask.IsValid()) {
      readerTask.SyncCancel();
    }
    if (writerTask.IsValid()) {
      writerTask.SyncCancel();
    }
  }

  stopTask.SyncCancel();
  context.Finish();
  return context.Failed() ? EXIT_FAILURE : EXIT_SUCCESS;
}
