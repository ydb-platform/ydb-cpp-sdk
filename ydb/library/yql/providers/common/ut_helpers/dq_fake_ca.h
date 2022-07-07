#pragma once

#include <ydb/library/yql/minikql/computation/mkql_computation_node_holders.h>
#include <ydb/library/yql/dq/actors/compute/dq_compute_actor_async_io.h>
#include <ydb/library/yql/dq/actors/protos/dq_events.pb.h>
#include <ydb/library/yql/dq/proto/dq_checkpoint.pb.h>
#include <ydb/library/yql/minikql/computation/mkql_value_builder.h>
#include <ydb/library/yql/minikql/mkql_alloc.h>
#include <ydb/library/yql/minikql/mkql_program_builder.h>

#include <ydb/core/testlib/basics/runtime.h>

#include <library/cpp/retry/retry.h>
#include <library/cpp/testing/unittest/registar.h>

#include <chrono>
#include <queue>

namespace NYql::NDq {

class TFakeActor;

using TRuntimePtr = std::unique_ptr<NActors::TTestActorRuntime>;
using TCallback = std::function<void(TFakeActor&)>;
template<typename T>
using TReadValueParser = std::function<std::vector<T>(const NUdf::TUnboxedValue&)>;
using TWriteValueProducer = std::function<NKikimr::NMiniKQL::TUnboxedValueVector(NKikimr::NMiniKQL::THolderFactory&)>;

namespace {
    struct TEvPrivate {
        // Event ids
        enum EEv : ui32 {
            EvBegin = EventSpaceBegin(NActors::TEvents::ES_PRIVATE),

            EvExecute = EvBegin,

            EvEnd
        };

        static_assert(EvEnd < EventSpaceEnd(NActors::TEvents::ES_PRIVATE), "expect EvEnd < EventSpaceEnd(NActors::TEvents::ES_PRIVATE)");

        // Events

        struct TEvExecute : public NActors::TEventLocal<TEvExecute, EvExecute> {
            TEvExecute(NThreading::TPromise<void>& promise, TCallback callback, std::exception_ptr& resultException)
                : Promise(promise)
                , Callback(callback)
                , ResultException(resultException)
            {}

            NThreading::TPromise<void> Promise;
            TCallback Callback;
            std::exception_ptr& ResultException;
        };
    };
}

struct TAsyncInputPromises {
    NThreading::TPromise<void> NewAsyncInputDataArrived = NThreading::NewPromise();
    NThreading::TPromise<TIssues> FatalError = NThreading::NewPromise<TIssues>();
};

struct TAsyncOutputPromises {
    NThreading::TPromise<void> ResumeExecution = NThreading::NewPromise();
    NThreading::TPromise<TIssues> Issue = NThreading::NewPromise<TIssues>();
    NThreading::TPromise<NDqProto::TSinkState> StateSaved = NThreading::NewPromise<NDqProto::TSinkState>();
};

NYql::NDqProto::TCheckpoint CreateCheckpoint(ui64 id = 0);

class TFakeActor : public NActors::TActor<TFakeActor> {
    struct TAsyncInputEvents {
        explicit TAsyncInputEvents(TFakeActor& parent) : Parent(parent) {}

        void OnNewAsyncInputDataArrived(ui64) {
            Parent.AsyncInputPromises.NewAsyncInputDataArrived.SetValue();
            Parent.AsyncInputPromises.NewAsyncInputDataArrived = NThreading::NewPromise();
        }

        void OnAsyncInputError(ui64, const TIssues& issues, bool isFatal) {
            Y_UNUSED(isFatal);
            Parent.AsyncInputPromises.FatalError.SetValue(issues);
            Parent.AsyncInputPromises.FatalError = NThreading::NewPromise<TIssues>();
        }

        TFakeActor& Parent;
    };

    struct TAsyncOutputCallbacks : public IDqComputeActorAsyncOutput::ICallbacks {
        explicit TAsyncOutputCallbacks(TFakeActor& parent) : Parent(parent) {}

        void ResumeExecution() override {
            Parent.AsyncOutputPromises.ResumeExecution.SetValue();
            Parent.AsyncOutputPromises.ResumeExecution = NThreading::NewPromise();
        };

        void OnAsyncOutputError(ui64, const TIssues& issues, bool isFatal) override {
            Y_UNUSED(isFatal);
            Parent.AsyncOutputPromises.Issue.SetValue(issues);
            Parent.AsyncOutputPromises.Issue = NThreading::NewPromise<TIssues>();
        };

        void OnAsyncOutputStateSaved(NDqProto::TSinkState&& state, ui64 outputIndex, const NDqProto::TCheckpoint&) override {
            Y_UNUSED(outputIndex);
            Parent.AsyncOutputPromises.StateSaved.SetValue(state);
            Parent.AsyncOutputPromises.StateSaved = NThreading::NewPromise<NDqProto::TSinkState>();
        };

        void OnAsyncOutputFinished(ui64 outputIndex) override {
            Y_UNUSED(outputIndex);
        }

        TFakeActor& Parent;
    };

public:
    TFakeActor(TAsyncInputPromises& sourcePromises, TAsyncOutputPromises& asyncOutputPromises);
    ~TFakeActor();

    void InitAsyncOutput(IDqComputeActorAsyncOutput* dqAsyncOutput, IActor* dqAsyncOutputAsActor);
    void InitAsyncInput(IDqComputeActorAsyncInput* dqAsyncInput, IActor* dqAsyncInputAsActor);
    void Terminate();

    TAsyncOutputCallbacks& GetAsyncOutputCallbacks();
    NKikimr::NMiniKQL::THolderFactory& GetHolderFactory();

public:
    IDqComputeActorAsyncInput* DqAsyncInput = nullptr;
    IDqComputeActorAsyncOutput* DqAsyncOutput = nullptr;

private:
    STRICT_STFUNC(StateFunc,
        hFunc(TEvPrivate::TEvExecute, Handle);
        hFunc(IDqComputeActorAsyncInput::TEvNewAsyncInputDataArrived, Handle);
        hFunc(IDqComputeActorAsyncInput::TEvAsyncInputError, Handle);
    )

    void Handle(TEvPrivate::TEvExecute::TPtr& ev) {
        TGuard<NKikimr::NMiniKQL::TScopedAlloc> guard(Alloc);
        try {
            ev->Get()->Callback(*this);
        } catch (...) {
            ev->Get()->ResultException = std::current_exception();
        }
        ev->Get()->Promise.SetValue();
    }

    void Handle(const IDqComputeActorAsyncInput::TEvNewAsyncInputDataArrived::TPtr& ev) {
        AsyncInputEvents.OnNewAsyncInputDataArrived(ev->Get()->InputIndex);
    }

    void Handle(const IDqComputeActorAsyncInput::TEvAsyncInputError::TPtr& ev) {
        AsyncInputEvents.OnAsyncInputError(ev->Get()->InputIndex, ev->Get()->Issues, ev->Get()->IsFatal);
    }

public:
    NKikimr::NMiniKQL::TScopedAlloc Alloc;
    NKikimr::NMiniKQL::TMemoryUsageInfo MemoryInfo;
    NKikimr::NMiniKQL::THolderFactory HolderFactory;

    NKikimr::NMiniKQL::TTypeEnvironment TypeEnv;
    TIntrusivePtr<NKikimr::NMiniKQL::IFunctionRegistry> FunctionRegistry;
    NKikimr::NMiniKQL::TProgramBuilder ProgramBuilder;
    NKikimr::NMiniKQL::TDefaultValueBuilder ValueBuilder;

private:
    std::optional<NActors::TActorId> DqAsyncInputActorId;
    IActor* DqAsyncInputAsActor = nullptr;

    std::optional<NActors::TActorId> DqAsyncOutputActorId;
    IActor* DqAsyncOutputAsActor = nullptr;

    TAsyncInputEvents AsyncInputEvents;
    TAsyncOutputCallbacks AsyncOutputCallbacks;

    TAsyncInputPromises& AsyncInputPromises;
    TAsyncOutputPromises& AsyncOutputPromises;
};

struct TFakeCASetup {
    TFakeCASetup();
    ~TFakeCASetup();

    template<typename T>
    std::vector<T> AsyncInputRead(const TReadValueParser<T> parser, i64 freeSpace = 12345) {
        std::vector<T> result;
        Execute([&result, &parser, freeSpace](TFakeActor& actor) {
            NKikimr::NMiniKQL::TUnboxedValueVector buffer;
            bool finished = false;
            actor.DqAsyncInput->GetAsyncInputData(buffer, finished, freeSpace);

            for (const auto& uv : buffer) {
                for (const auto item : parser(uv)) {
                    result.emplace_back(item);
                }
            }
        });

        return result;
    }

    template<typename T>
    std::vector<T> AsyncInputReadUntil(
        const TReadValueParser<T> parser,
        ui64 size,
        i64 eachReadFreeSpace = 1000,
        TDuration timeout = TDuration::Seconds(10))
    {
        std::vector<T> result;
        DoWithRetry([&](){
                auto batch = AsyncInputRead<T>(parser, eachReadFreeSpace);
                for (const auto& item : batch) {
                    result.emplace_back(item);
                }

                if (result.size() < size) {
                    AsyncInputPromises.NewAsyncInputDataArrived.GetFuture().Wait(timeout);
                    ythrow yexception() << "Not enough data";
                }
            },
            TRetryOptions(3),
            false);

        return result;
    }

    void AsyncOutputWrite(const TWriteValueProducer valueProducer, TMaybe<NDqProto::TCheckpoint> checkpoint = Nothing(), bool finish = false);

    void SaveSourceState(NDqProto::TCheckpoint checkpoint, NDqProto::TSourceState& state);

    void LoadSource(const NDqProto::TSourceState& state);
    void LoadSink(const NDqProto::TSinkState& state);

    void Execute(TCallback callback);

public:
    TRuntimePtr Runtime;
    NActors::TActorId FakeActorId;
    TAsyncInputPromises AsyncInputPromises;
    TAsyncOutputPromises AsyncOutputPromises;
};

} // namespace NKikimr::NMiniKQL
