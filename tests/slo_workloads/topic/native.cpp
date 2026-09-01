#include "topic.h"

#include <library/cpp/threading/future/core/coroutine_traits.h>
#include <library/cpp/threading/future/wait/wait.h>

#include <util/string/builder.h>
#include <util/string/cast.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
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
using namespace std::chrono_literals;

class TStopFuture {
private:
  struct TCallback {
    NThreading::TPromise<void> Promise;

    void operator()() noexcept { Promise.TrySetValue(); }
  };

public:
  explicit TStopFuture(std::stop_token stopToken)
      : Promise_(NThreading::NewPromise<void>()),
        Callback_(stopToken, TCallback{Promise_}) {}

  NThreading::TFuture<void> GetFuture() const { return Promise_.GetFuture(); }

private:
  NThreading::TPromise<void> Promise_;
  std::stop_callback<TCallback> Callback_;
};

class TAsyncTimer {
public:
  TAsyncTimer()
      : Thread_([this](std::stop_token stopToken) { Run(stopToken); }) {}

  ~TAsyncTimer() {
    Thread_.request_stop();
    Condition_.notify_all();
  }

  NThreading::TFuture<bool> WaitUntil(TClock::time_point deadline) {
    auto promise = NThreading::NewPromise<bool>();
    auto future = promise.GetFuture();
    if (deadline <= TClock::now()) {
      promise.SetValue(true);
      return future;
    }

    {
      std::lock_guard lock(Mutex_);
      Queue_.push(TEntry{deadline, NextId_++, std::move(promise)});
    }
    Condition_.notify_all();
    return future;
  }

private:
  struct TEntry {
    TClock::time_point Deadline;
    std::uint64_t Id;
    NThreading::TPromise<bool> Promise;
  };

  struct TEntryLater {
    bool operator()(const TEntry &lhs, const TEntry &rhs) const {
      return lhs.Deadline > rhs.Deadline ||
             (lhs.Deadline == rhs.Deadline && lhs.Id > rhs.Id);
    }
  };

  void Run(std::stop_token stopToken) {
    std::unique_lock lock(Mutex_);
    while (!stopToken.stop_requested()) {
      if (Queue_.empty()) {
        Condition_.wait(lock, stopToken, [this] { return !Queue_.empty(); });
        continue;
      }

      const auto deadline = Queue_.top().Deadline;
      Condition_.wait_until(lock, stopToken, deadline, [this, deadline] {
        return Queue_.empty() || Queue_.top().Deadline < deadline;
      });
      if (stopToken.stop_requested()) {
        break;
      }
      if (Queue_.empty() || Queue_.top().Deadline > TClock::now()) {
        continue;
      }

      std::vector<NThreading::TPromise<bool>> ready;
      const auto now = TClock::now();
      while (!Queue_.empty() && Queue_.top().Deadline <= now) {
        ready.push_back(Queue_.top().Promise);
        Queue_.pop();
      }

      lock.unlock();
      for (auto &promise : ready) {
        promise.TrySetValue(true);
      }
      lock.lock();
    }

    std::vector<NThreading::TPromise<bool>> cancelled;
    while (!Queue_.empty()) {
      cancelled.push_back(Queue_.top().Promise);
      Queue_.pop();
    }
    lock.unlock();
    for (auto &promise : cancelled) {
      promise.TrySetValue(false);
    }
  }

  std::mutex Mutex_;
  std::condition_variable_any Condition_;
  std::priority_queue<TEntry, std::vector<TEntry>, TEntryLater> Queue_;
  std::uint64_t NextId_ = 0;
  std::jthread Thread_;
};

class TTopicRateLimiter {
public:
  TTopicRateLimiter(TAsyncTimer &timer, std::uint32_t rps)
      : Timer_(timer),
        IntervalMicros_(std::max<std::uint64_t>(1, 1'000'000 / rps)),
        NextMicros_(NowMicros()) {}

  NThreading::TFuture<bool>
  Acquire(const NThreading::TFuture<void> &stopFuture) {
    std::int64_t current = NextMicros_.load();
    std::int64_t scheduled;
    do {
      scheduled = std::max(current, NowMicros());
    } while (!NextMicros_.compare_exchange_weak(current,
                                                scheduled + IntervalMicros_));

    auto timerFuture = Timer_.WaitUntil(
        TClock::time_point(std::chrono::microseconds(scheduled)));
    co_await NThreading::WaitAny(timerFuture.IgnoreResult(), stopFuture);
    if (stopFuture.IsReady()) {
      co_return false;
    }
    co_return co_await timerFuture;
  }

private:
  static std::int64_t NowMicros() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
               TClock::now().time_since_epoch())
        .count();
  }

  TAsyncTimer &Timer_;
  const std::int64_t IntervalMicros_;
  std::atomic<std::int64_t> NextMicros_;
};

NThreading::TFuture<void> RequestStopAt(TAsyncTimer &timer,
                                        TClock::time_point deadline,
                                        std::stop_source stopSource) {
  if (co_await timer.WaitUntil(deadline)) {
    stopSource.request_stop();
  }
}

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
    TStopFuture stop(stopToken);
    const auto stopFuture = stop.GetFuture();
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
          if (!co_await Limiter_.Acquire(stopFuture)) {
            break;
          }

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
        co_await NThreading::WaitAny(eventFuture, stopFuture);
        if (stopToken.stop_requested()) {
          break;
        }
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
    TStopFuture stop(stopToken);
    const auto stopFuture = stop.GetFuture();
    auto &session = Sessions_[readerIndex];

    try {
      session = Client_.CreateReadSession(
          MakeTopicReadSettings(Context_.GetOptions()));
      while (!stopToken.stop_requested()) {
        auto eventFuture = session->WaitEvent();
        co_await NThreading::WaitAny(eventFuture, stopFuture);
        if (stopToken.stop_requested()) {
          break;
        }
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

  TAsyncTimer timer;
  std::stop_source stopSource;
  TTopicClient client(dbOptions.Driver);
  TTopicRunContext context(options, stopSource);
  TTopicRateLimiter limiter(timer, options.WriteRps);
  TTopicReaders readers(client, context);
  TTopicWriters writers(client, context, limiter);

  context.Start();
  const auto startedAt = TClock::now();
  const auto stopAt = startedAt + std::chrono::seconds(options.SecondsToRun);
  const auto hardDeadline = stopAt + 60s;

  auto stopRequest = RequestStopAt(timer, stopAt, stopSource);
  auto readerFuture = readers.Run(stopSource.get_token());
  auto writerFuture = writers.Run(stopSource.get_token());
  auto workers = NThreading::WaitAll(readerFuture, writerFuture);
  auto watchdog = timer.WaitUntil(hardDeadline);
  auto finished = NThreading::WaitAny(workers, watchdog.IgnoreResult());

  finished.GetValueSync();
  if (!workers.IsReady()) {
    context.Fail("topic workers did not stop within 60 seconds");
    stopSource.request_stop();
  }
  workers.GetValueSync();
  Y_UNUSED(stopRequest);

  writers.Close();
  readers.Close();
  context.Finish();
  return context.Failed() ? EXIT_FAILURE : EXIT_SUCCESS;
}
