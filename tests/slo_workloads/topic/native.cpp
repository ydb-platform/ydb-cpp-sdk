#include "topic.h"

#include <library/cpp/threading/future/core/coroutine_traits.h>
#include <library/cpp/threading/future/wait/wait.h>

#include <util/string/builder.h>
#include <util/string/cast.h>

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <stop_token>
#include <thread>
#include <utility>
#include <vector>

using namespace NYdb::NTopic;

namespace {

using TClock = std::chrono::steady_clock;

class TTopicRateLimiter {
public:
  explicit TTopicRateLimiter(std::uint32_t rps)
      : Interval_(std::chrono::microseconds(
            std::max<std::uint64_t>(1, 1'000'000 / rps))),
        Thread_([this] { Run(); }) {}

  ~TTopicRateLimiter() {
    {
      std::lock_guard lock(Mutex_);
      Stopping_ = true;
    }
    Condition_.notify_all();
    Thread_.join();
  }

  NThreading::TFuture<void> Acquire() {
    auto promise = NThreading::NewPromise<void>();
    auto future = promise.GetFuture();
    {
      std::lock_guard lock(Mutex_);
      Waiters_.push(std::move(promise));
    }
    Condition_.notify_one();
    co_await future;
  }

private:
  void Run() {
    auto next = TClock::now();
    std::unique_lock lock(Mutex_);

    while (!Stopping_) {
      Condition_.wait(lock, [this] { return Stopping_ || !Waiters_.empty(); });
      if (Stopping_) {
        break;
      }

      auto promise = std::move(Waiters_.front());
      Waiters_.pop();
      next = std::max(next, TClock::now());
      const auto wakeAt = next;
      next += Interval_;

      lock.unlock();
      std::this_thread::sleep_until(wakeAt);
      promise.TrySetValue();
      lock.lock();
    }

    std::vector<NThreading::TPromise<void>> cancelled;
    while (!Waiters_.empty()) {
      cancelled.push_back(std::move(Waiters_.front()));
      Waiters_.pop();
    }
    lock.unlock();
    for (auto &promise : cancelled) {
      promise.TrySetValue();
    }
  }

  const std::chrono::microseconds Interval_;
  std::mutex Mutex_;
  std::condition_variable Condition_;
  std::queue<NThreading::TPromise<void>> Waiters_;
  bool Stopping_ = false;
  std::thread Thread_;
};

class TTopicWriters {
public:
  TTopicWriters(TTopicClient &client, TTopicRunContext &context,
                TTopicRateLimiter &limiter)
      : Client_(client), Context_(context), Limiter_(limiter),
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
    std::optional<std::uint64_t> pendingSeqNo;
    bool ownFailure = false;

    try {
      session = Client_.CreateWriteSession(
          MakeTopicWriteSettings(Context_.GetOptions(), writerIndex));
      std::uint64_t nextSeqNo = 1;

      while (!stopToken.stop_requested()) {
        if (continuationToken && !pendingSeqNo) {
          co_await Limiter_.Acquire();

          const std::uint64_t seqNo = nextSeqNo++;
          TWriteMessage message(ToString(seqNo));
          message.SeqNo(seqNo).CreateTimestamp(TInstant::Now());
          writeStat = Context_.StartWrite();
          session->Write(std::move(*continuationToken), std::move(message));
          continuationToken.reset();
          pendingSeqNo = seqNo;
          continue;
        }

        auto eventFuture = session->WaitEvent();
        co_await eventFuture;

        bool acked = false;
        for (auto &event : session->GetEvents(false)) {
          if (!HandleTopicWriteEvent(event, continuationToken, pendingSeqNo,
                                     acked, Context_)) {
            ownFailure = true;
            break;
          }
        }
        if (acked && writeStat) {
          Context_.FinishWrite(writeStat, true);
          writeStat.reset();
          pendingSeqNo.reset();
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
  TTopicRateLimiter &Limiter_;
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
      session = Client_.CreateReadSession(
          MakeTopicReadSettings(Context_.GetOptions()));
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
  TTopicRateLimiter limiter(options.WriteRps);
  TTopicReaders readers(client, context);
  TTopicWriters writers(client, context, limiter);

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
