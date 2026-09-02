#include "topic.h"

#include <library/cpp/threading/future/core/coroutine_traits.h>
#include <library/cpp/threading/future/wait/wait.h>

#include <util/string/builder.h>

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <stop_token>
#include <thread>
#include <utility>
#include <vector>

using namespace NYdb::NTopic;

namespace {

class TTopicWriters {
public:
  TTopicWriters(TTopicClient &client, TTopicRunContext &context)
      : Client_(client), Context_(context),
        Sessions_(context.GetOptions().WriterCount) {}

  NThreading::TFuture<void> Run(std::stop_token stopToken) {
    std::vector<NThreading::TFuture<void>> writers;
    writers.reserve(Context_.GetOptions().WriterCount);
    for (std::uint32_t i = 0; i < Context_.GetOptions().WriterCount; ++i) {
      writers.push_back(RunWriter(i, stopToken));
    }
    co_await NThreading::WaitAll(writers);
  }

  void Close() {
    for (auto &session : Sessions_) {
      if (session) {
        try {
          session->Close(TDuration::Zero());
        } catch (const std::exception &) {
        }
        session.reset();
      }
    }
  }

private:
  NThreading::TFuture<void> RunWriter(std::uint32_t writerIndex,
                                      std::stop_token stopToken) {
    auto &session = Sessions_[writerIndex];
    std::shared_ptr<TStatUnit> writeStat;
    std::optional<TContinuationToken> continuationToken;
    bool ownFailure = false;

    try {
      session = Client_.CreateWriteSession(
          MakeTopicWriteSettings(Context_.GetOptions(), writerIndex,
                                 [this] { Context_.RecordWriteRetry(); }));

      while (!stopToken.stop_requested()) {
        if (continuationToken && !writeStat) {
          writeStat = Context_.StartWrite();
          session->Write(std::move(*continuationToken),
                         TWriteMessage("message"));
          continuationToken.reset();
          continue;
        }

        auto eventFuture = session->WaitEvent();
        co_await eventFuture;

        bool acked = false;
        for (auto &event : session->GetEvents(false)) {
          if (!HandleTopicWriteEvent(event, continuationToken, acked,
                                     Context_)) {
            ownFailure = true;
            break;
          }
        }
        if (acked && !writeStat) {
          ownFailure = true;
          Context_.Fail("write session acknowledged no pending message");
          break;
        } else if (acked) {
          Context_.FinishWrite(writeStat, true);
          writeStat.reset();
        }
        if (ownFailure) {
          break;
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
  }

  TTopicClient &Client_;
  TTopicRunContext &Context_;
  std::vector<std::shared_ptr<IWriteSession>> Sessions_;
};

class TTopicReaders {
public:
  TTopicReaders(TTopicClient &client, TTopicRunContext &context)
      : Client_(client), Context_(context),
        Sessions_(context.GetOptions().ReaderCount) {}

  NThreading::TFuture<void> Run(std::stop_token stopToken) {
    std::vector<NThreading::TFuture<void>> readers;
    readers.reserve(Context_.GetOptions().ReaderCount);
    for (std::uint32_t i = 0; i < Context_.GetOptions().ReaderCount; ++i) {
      readers.push_back(RunReader(i, stopToken));
    }
    co_await NThreading::WaitAll(readers);
  }

  void Close() {
    for (auto &session : Sessions_) {
      if (session) {
        try {
          session->Close(TDuration::Zero());
        } catch (const std::exception &) {
        }
        session.reset();
      }
    }
  }

private:
  NThreading::TFuture<void> RunReader(std::uint32_t readerIndex,
                                      std::stop_token stopToken) {
    auto &session = Sessions_[readerIndex];

    try {
      session = Client_.CreateReadSession(MakeTopicReadSettings(
          Context_.GetOptions(), [this] { Context_.RecordReadRetry(); }));
      while (!stopToken.stop_requested()) {
        auto eventFuture = session->WaitEvent();
        co_await eventFuture;

        for (auto &event : session->GetEvents(false)) {
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
  }

  TTopicClient &Client_;
  TTopicRunContext &Context_;
  std::vector<std::shared_ptr<IReadSession>> Sessions_;
};

} // namespace

int DoRun(TDatabaseOptions &dbOptions, int argc, char **argv) {
  TTopicOptions options(dbOptions);
  if (!ParseTopicOptions(argc, argv, options)) {
    return EXIT_FAILURE;
  }

  std::stop_source stopSource;
  TTopicClient client(dbOptions.Driver);
  TTopicRunContext context(options, stopSource);
  TTopicReaders readers(client, context);
  TTopicWriters writers(client, context);

  context.Start();
  std::thread([stopSource, duration = std::chrono::seconds(
                               options.SecondsToRun)]() mutable {
    std::this_thread::sleep_for(duration);
    stopSource.request_stop();
  }).detach();

  auto readerFuture = readers.Run(stopSource.get_token());
  auto writerFuture = writers.Run(stopSource.get_token());
  auto workers = NThreading::WaitAll(readerFuture, writerFuture);
  workers.GetValueSync();

  writers.Close();
  readers.Close();
  context.Finish();
  return context.Failed() ? EXIT_FAILURE : EXIT_SUCCESS;
}
