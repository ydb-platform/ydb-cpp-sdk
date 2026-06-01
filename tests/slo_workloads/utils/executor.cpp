#include "executor.h"

const TDuration WaitTimeout = TDuration::Seconds(10);

// Debug use only:
std::atomic<std::uint64_t> ReadPromises = 0;
std::atomic<std::uint64_t> ExecutorPromises = 0;


TInsistentClient::TInsistentClient(const TCommonOptions& opts)
    : Client(
        opts.DatabaseOptions.Driver,
        NYdb::NTable::TClientSettings()
            .SessionPoolSettings(NYdb::NTable::TSessionPoolSettings().MaxActiveSessions(opts.MaxInfly))
            .MinSessionCV(8)
            .AllowRequestMigration(true)
    )
    , ClientMaxRetries(opts.MaxRetries)
    , SessionTimeout(opts.ReactionTime + ReactionTimeDelay)
{
}

TInsistentClient::~TInsistentClient() {
    Client.Stop().Wait(WaitTimeout);
}

void TInsistentClient::Report(TStringBuilder& out) const {
    out << "Operations dispatched: " << CounterStart.load()
        << ", succeeded: " << CounterOk.load() << Endl;
}

std::uint64_t TInsistentClient::GetActiveSessions() const {
    std::int64_t sessions = Client.GetActiveSessionCount();
    return static_cast<std::uint64_t>(sessions);
}

TAsyncFinalStatus TInsistentClient::ExecuteWithRetry(const NYdb::NTable::TTableClient::TOperationFunc& operation,
    const std::shared_ptr<TStatUnit>& stat)
{
    TTracedPromise<TFinalStatus> promise = TTracedPromise<TFinalStatus>(
        NThreading::NewPromise<TFinalStatus>(),
        &ExecutorPromises
    );

    CounterStart.fetch_add(1);

    NYdb::NTable::TRetryOperationSettings settings;
    settings.MaxRetries(ClientMaxRetries);
    settings.GetSessionClientTimeout(SessionTimeout);

    auto wrappedOperation = [operation, stat](NYdb::NTable::TSession session) {
        stat->IncRetryAttempts();
        return operation(session);
    };
    auto future = Client.RetryOperation(wrappedOperation, settings);
    future.Subscribe([promise, this](const NYdb::TAsyncStatus& f) mutable {
        Y_ABORT_UNLESS(f.HasValue());
        const auto& status = f.GetValue();
        if (status.IsSuccess()) {
            CounterOk.fetch_add(1);
        }
        promise.SetValue(status);
    });

    return promise.GetFuture();
}

TExecutor::TExecutor(const TCommonOptions& opts, TStat& stats, EMode mode)
    : Opts(opts)
    , Stats(stats)
    , InsistentClient(opts)
    , Semaphore(opts.MaxInputThreads)
    , Mode(mode)
{
    InputQueue = std::make_unique<TThreadPool>();
    InputQueue->Start(opts.MaxInputThreads);

    auto threadFunc = [this]() {
        TInstant wakeupTime = TInstant::Now();
        while (!ShouldStop.load()) {
            with_lock(Lock) {
                UpdateStats();
            }
            wakeupTime += TDuration::Seconds(1);
            SleepUntil(wakeupTime);
        }
    };
    MetricsPusherThread.reset(SystemThreadFactory()->Run(threadFunc).Release());
}

TExecutor::~TExecutor() {
    std::uint32_t infly = StopAndWait(WaitTimeout);
    if (MetricsPusherThread) {
        MetricsPusherThread->Join();
    } else {
        Cerr << (TStringBuilder() << "TExecutor::~TExecutor Error: MetricsPusherThread is not running." << Endl);
    }
    if (infly) {
        Cerr << "Warning: destroying TExecutor while having " << infly << " infly requests." << Endl;
    }
}

namespace {
    class TSemaphoreWrapper : public TThrRefBase {
    public:
        TSemaphoreWrapper(TFastSemaphore& semaphore)
            : Semaphore(semaphore)
        {}

        ~TSemaphoreWrapper() {
            if (Acquired) {
                Semaphore.Release();
            }
        }

        void Acquire() {
            Semaphore.Acquire();
            Acquired = true;
        }

    private:
        TFastSemaphore& Semaphore;
        bool Acquired = false;
    };
}

bool TExecutor::Execute(const NYdb::NTable::TTableClient::TOperationFunc& func) {
    TIntrusivePtr<TSemaphoreWrapper> SemaphoreWrapper;
    if (Mode == ModeBlocking) {
        SemaphoreWrapper = new TSemaphoreWrapper(Semaphore);
    }
    auto threadFunc = [this, func, SemaphoreWrapper]() {
        if (IsStopped()) {
            DecrementWaiting();
            return;
        }

        with_lock(Lock) {
            --Waiting;
            if (Infly < Opts.MaxInfly) {
                ++Infly;
                UpdateStats();
                if (Infly > MaxInfly) {
                    MaxInfly = Infly;
                }
            } else {
                Stats.ReportMaxInfly();
                UpdateStats();
                return;
            }
        }

        auto stat = Stats.StartRequest();

        auto future = InsistentClient.ExecuteWithRetry(func, stat);

        future.Subscribe([this, stat, SemaphoreWrapper](const TAsyncFinalStatus& future) mutable {
            Y_ABORT_UNLESS(future.HasValue());
            TFinalStatus resultStatus = future.GetValue();
            Stats.FinishRequest(stat, resultStatus);
            if (resultStatus) {
                CheckForError(*resultStatus);
            }
            DecrementInfly();
        });
    };

    if (IsStopped()) {
        return false;
    }

    bool CanLaunchJob = false;

    with_lock(Lock) {
        if (!AllJobsLaunched) {
            CanLaunchJob = true;
            ++Waiting;
        }
    }

    if (CanLaunchJob) {
        if (Mode == ModeBlocking) {
            SemaphoreWrapper->Acquire();
        }
        if (!InputQueue->AddFunc(threadFunc)) {
            DecrementWaiting();
        }
    }
    ++InProgressCount;
    InProgressSum += InputQueue->Size();
    return true;
}

void TExecutor::Start(TInstant deadline) {
    Deadline = deadline;
}

void TExecutor::Stop() {
    if (!IsStopped()) {
        ShouldStop.store(true);
        Finish();
    }
}

void TExecutor::Wait() {
    AllJobsFinished.WaitI();
    InputQueue->Stop();
}

std::uint32_t TExecutor::Wait(TDuration waitTimeout) {
    AllJobsFinished.WaitT(waitTimeout);
    InputQueue->Stop();
    return Infly;
}

void TExecutor::StopAndWait() {
    Stop();
    Wait();
}

std::uint32_t TExecutor::StopAndWait(TDuration waitTimeout) {
    Stop();
    return Wait(waitTimeout);
}

bool TExecutor::IsStopped() {
    return ShouldStop.load();
}

void TExecutor::Finish() {
    with_lock(Lock) {
        if (!AllJobsLaunched) {
            AllJobsLaunched = true;
            CheckForFinish();
        }
    }
}

void TExecutor::UpdateStats() {
    std::uint64_t activeSessions = InsistentClient.GetActiveSessions();
    if (activeSessions > MaxSecSessions) {
        MaxSecSessions = activeSessions;
    }

    // Debug use only:
    std::uint64_t readPromises = ReadPromises.load();
    if (readPromises > MaxSecReadPromises) {
        MaxSecReadPromises = readPromises;
    }
    std::uint64_t executorPromises = ExecutorPromises.load();
    if (executorPromises > MaxSecExecutorPromises) {
        MaxSecExecutorPromises = executorPromises;
    }
    ReportStats();
}

void TExecutor::ReportStats() {
    Stats.ReportStats(MaxSecSessions, MaxSecReadPromises, MaxSecExecutorPromises);
    TInstant now = TInstant::Now();
    if (now.Seconds() > LastReportSec) {
        Stats.ReportStats(MaxSecSessions, MaxSecReadPromises, MaxSecExecutorPromises);
        MaxSecSessions = 0;
        MaxSecReadPromises = 0;
        MaxSecExecutorPromises = 0;
        LastReportSec = now.Seconds();
    }
}

void TExecutor::DecrementInfly() {
    with_lock(Lock) {
        --Infly;
        UpdateStats();
        if (!Infly) {
            CheckForFinish();
        }
    }
}

void TExecutor::DecrementWaiting() {
    with_lock(Lock) {
        --Waiting;
        CheckForFinish();
    }
}

void TExecutor::CheckForFinish() {
    if (AllJobsLaunched && !Infly && !Waiting) {
        AllJobsFinished.Signal();
    }
}

void TExecutor::CheckForError(const NYdb::TStatus& status) {
    if (!status.IsSuccess()) {
        with_lock(ErrorLock) {
            auto it = Errors.find(status.GetStatus());
            if (it == Errors.end()) {
                Errors.insert({ status.GetStatus() , { status.GetIssues().ToString() , 1 } });
            } else {
                ++it->second.Counter;
            }
        }
    }
}

void TExecutor::Report(TStringBuilder& out) const {
    out << MaxInfly << " maxInfly" << Endl
        << (InProgressCount ? InProgressSum / InProgressCount : 0) << " average Inprogress threads in input queue" << Endl;
    InsistentClient.Report(out);
    with_lock(ErrorLock) {
        if (Errors.size()) {
            out << "Errors:" << Endl;
            for (auto& error : Errors) {
                out << error.second.Counter << " errors with status " << error.first << ": " << error.second.Message << Endl;
            }
        }
    }
}
