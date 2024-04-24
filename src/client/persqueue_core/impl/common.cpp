#include "common.h"

#include <src/util/charset/unidata.h>

namespace NYdb::NPersQueue {

ERetryErrorClass GetRetryErrorClass(EStatus status) {
    switch (status) {
    case EStatus::SUCCESS:          // NoRetry?
    case EStatus::INTERNAL_ERROR:   // NoRetry?
    case EStatus::ABORTED:
    case EStatus::UNAVAILABLE:
    case EStatus::GENERIC_ERROR:    // NoRetry?
    case EStatus::BAD_SESSION:      // NoRetry?
    case EStatus::SESSION_EXPIRED:
    case EStatus::CANCELLED:
    case EStatus::UNDETERMINED:
    case EStatus::SESSION_BUSY:
    case EStatus::CLIENT_INTERNAL_ERROR:
    case EStatus::CLIENT_CANCELLED:
    case EStatus::CLIENT_OUT_OF_RANGE:
        return ERetryErrorClass::ShortRetry;

    case EStatus::OVERLOADED:
    case EStatus::TIMEOUT:
    case EStatus::TRANSPORT_UNAVAILABLE:
    case EStatus::CLIENT_RESOURCE_EXHAUSTED:
    case EStatus::CLIENT_DEADLINE_EXCEEDED:
    case EStatus::CLIENT_LIMITS_REACHED:
    case EStatus::CLIENT_DISCOVERY_FAILED:
        return ERetryErrorClass::LongRetry;

    case EStatus::SCHEME_ERROR:
    case EStatus::STATUS_UNDEFINED:
    case EStatus::BAD_REQUEST:
    case EStatus::UNAUTHORIZED:
    case EStatus::PRECONDITION_FAILED:
    case EStatus::UNSUPPORTED:
    case EStatus::ALREADY_EXISTS:
    case EStatus::NOT_FOUND:
    case EStatus::EXTERNAL_ERROR:
    case EStatus::CLIENT_UNAUTHENTICATED:
    case EStatus::CLIENT_CALL_UNIMPLEMENTED:
        return ERetryErrorClass::NoRetry;
    }
}

ERetryErrorClass GetRetryErrorClassV2(EStatus status) {
    switch (status) {
        case EStatus::SCHEME_ERROR:
            return ERetryErrorClass::NoRetry;
        default:
            return GetRetryErrorClass(status);

    }
}

std::string IssuesSingleLineString(const NYql::TIssues& issues) {
    return SubstGlobalCopy(issues.ToString(), '\n', ' ');
}

void Cancel(NYdbGrpc::IQueueClientContextPtr& context) {
    if (context) {
        context->Cancel();
    }
}

NYql::TIssues MakeIssueWithSubIssues(const std::string& description, const NYql::TIssues& subissues) {
    NYql::TIssues issues;
    NYql::TIssue issue(description);
    for (const NYql::TIssue& i : subissues) {
        issue.AddSubIssue(MakeIntrusive<NYql::TIssue>(i));
    }
    issues.AddIssue(std::move(issue));
    return issues;
}

static std::string_view SplitPort(std::string_view endpoint) {
    for (int i = endpoint.size() - 1; i >= 0; --i) {
        if (endpoint[i] == ':') {
            return endpoint.substr(i + 1, std::string_view::npos);
        }
        if (!IsDigit(endpoint[i])) {
            return std::string_view(); // empty
        }
    }
    return std::string_view(); // empty
}

std::string ApplyClusterEndpoint(std::string_view driverEndpoint, const std::string& clusterDiscoveryEndpoint) {
    const std::string_view clusterDiscoveryPort = SplitPort(clusterDiscoveryEndpoint);
    if (!clusterDiscoveryPort.empty()) {
        return clusterDiscoveryEndpoint;
    }

    const std::string_view driverPort = SplitPort(driverEndpoint);
    if (driverPort.empty()) {
        return clusterDiscoveryEndpoint;
    }

    const bool hasColon = clusterDiscoveryEndpoint.find(':') != std::string::npos;
    if (hasColon) {
        return TStringBuilder() << '[' << clusterDiscoveryEndpoint << "]:" << driverPort;
    } else {
        return TStringBuilder() << clusterDiscoveryEndpoint << ':' << driverPort;
    }
}

void IAsyncExecutor::Post(TFunction&& f) {
    PostImpl(std::move(f));
}

IAsyncExecutor::TPtr CreateDefaultExecutor() {
    return CreateThreadPoolExecutor(1);
}

void TThreadPoolExecutor::PostImpl(std::vector<TFunction>&& fs) {
    for (auto& f : fs) {
        ThreadPool->SafeAddFunc(std::move(f));
    }
}

void TThreadPoolExecutor::PostImpl(TFunction&& f) {
    ThreadPool->SafeAddFunc(std::move(f));
}

TSerialExecutor::TSerialExecutor(IAsyncExecutor::TPtr executor)
    : Executor(executor)
{
    Y_ABORT_UNLESS(executor);
}

void TSerialExecutor::PostImpl(std::vector<TFunction>&& fs) {
    for (auto& f : fs) {
        PostImpl(std::move(f));
    }
}

void TSerialExecutor::PostImpl(TFunction&& f) {
    {
        std::lock_guard<std::mutex> guard(Mutex);
        ExecutionQueue.push(std::move(f));
        if (Busy) {
            return;
        }
        PostNext();
    }
}

void TSerialExecutor::PostNext() {
    Y_ABORT_UNLESS(!Busy);

    if (ExecutionQueue.empty()) {
        return;
    }

    auto weakThis = weak_from_this();
    Executor->Post([weakThis, f = std::move(ExecutionQueue.front())]() {
        if (auto sharedThis = weakThis.lock()) {
            f();
            {
                std::lock_guard<std::mutex> guard(sharedThis->Mutex);
                sharedThis->Busy = false;
                sharedThis->PostNext();
            }
        }
    });
    ExecutionQueue.pop();
    Busy = true;
}

IExecutor::TPtr CreateThreadPoolExecutor(size_t threads) {
    return MakeIntrusive<TThreadPoolExecutor>(threads);
}

IExecutor::TPtr CreateGenericExecutor() {
    return CreateThreadPoolExecutor(1);
}

IExecutor::TPtr CreateThreadPoolExecutorAdapter(std::shared_ptr<IThreadPool> threadPool) {
    return MakeIntrusive<TThreadPoolExecutor>(std::move(threadPool));
}

TThreadPoolExecutor::TThreadPoolExecutor(std::shared_ptr<IThreadPool> threadPool)
    : ThreadPool(std::move(threadPool))
{
    IsFakeThreadPool = dynamic_cast<TFakeThreadPool*>(ThreadPool.get()) != nullptr;
}

TThreadPoolExecutor::TThreadPoolExecutor(size_t threadsCount)
    : TThreadPoolExecutor(CreateThreadPool(threadsCount))
{
    Y_ABORT_UNLESS(threadsCount > 0);
    ThreadsCount = threadsCount;
}

IExecutor::TPtr CreateSyncExecutor()
{
    return MakeIntrusive<TSyncExecutor>();
}

}
