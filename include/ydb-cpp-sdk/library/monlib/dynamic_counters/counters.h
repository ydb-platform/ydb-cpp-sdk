#pragma once

#include <ydb-cpp-sdk/library/monlib/counters/counters.h>
#include <ydb-cpp-sdk/library/monlib/metrics/histogram_collector.h>

#include <ydb-cpp-sdk/library/containers/stack_vector/stack_vec.h>

#include <ydb-cpp-sdk/util/generic/cast.h>
#include <ydb-cpp-sdk/util/generic/ptr.h>
#include <ydb-cpp-sdk/util/string/cast.h>
#include <ydb-cpp-sdk/util/system/rwlock.h>

#include <map>
#include <functional>

namespace NMonitoring {
    struct TCounterForPtr;
    struct TDynamicCounters;
    struct ICountableConsumer;


    struct TCountableBase: public TAtomicRefCount<TCountableBase> {
        // Private means that the object must not be serialized unless the consumer
        // has explicitly specified this by setting its Visibility to Private.
        //
        // Works only for the methods that accept ICountableConsumer
        enum class EVisibility: ui8 {
            Unspecified,
            Public,
            Private,
        };

        virtual ~TCountableBase() {
        }

        virtual void Accept(
            const std::string& labelName, const std::string& labelValue,
            ICountableConsumer& consumer) const = 0;

        virtual EVisibility Visibility() const {
            return Visibility_;
        }

    protected:
        EVisibility Visibility_{EVisibility::Unspecified};
    };

    inline bool IsVisible(TCountableBase::EVisibility myLevel, TCountableBase::EVisibility consumerLevel) {
        if (myLevel == TCountableBase::EVisibility::Private
            && consumerLevel != TCountableBase::EVisibility::Private) {

            return false;
        }

        return true;
    }

    struct ICountableConsumer {
        virtual ~ICountableConsumer() {
        }

        virtual void OnCounter(
            const std::string& labelName, const std::string& labelValue,
            const TCounterForPtr* counter) = 0;

        virtual void OnHistogram(
            const std::string& labelName, const std::string& labelValue,
            IHistogramSnapshotPtr snapshot, bool derivative) = 0;

        virtual void OnGroupBegin(
            const std::string& labelName, const std::string& labelValue,
            const TDynamicCounters* group) = 0;

        virtual void OnGroupEnd(
            const std::string& labelName, const std::string& labelValue,
            const TDynamicCounters* group) = 0;

        virtual TCountableBase::EVisibility Visibility() const {
            return TCountableBase::EVisibility::Unspecified;
        }
    };

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4522) // multiple assignment operators specified
#endif                          // _MSC_VER

    struct TCounterForPtr: public TDeprecatedCounter, public TCountableBase {
        TCounterForPtr(bool derivative = false, EVisibility vis = EVisibility::Public)
            : TDeprecatedCounter(0ULL, derivative)
        {
            Visibility_ = vis;
        }

        TCounterForPtr(const TCounterForPtr&) = delete;
        TCounterForPtr& operator=(const TCounterForPtr& other) = delete;

        void Accept(
            const std::string& labelName, const std::string& labelValue,
            ICountableConsumer& consumer) const override {
            if (IsVisible(Visibility(), consumer.Visibility())) {
                consumer.OnCounter(labelName, labelValue, this);
            }
        }

        TCountableBase::EVisibility Visibility() const override {
            return Visibility_;
        }

        using TDeprecatedCounter::operator++;
        using TDeprecatedCounter::operator--;
        using TDeprecatedCounter::operator+=;
        using TDeprecatedCounter::operator-=;
        using TDeprecatedCounter::operator=;
        using TDeprecatedCounter::operator!;
    };

    struct TExpiringCounter: public TCounterForPtr {
        explicit TExpiringCounter(bool derivative = false, EVisibility vis = EVisibility::Public)
            : TCounterForPtr{derivative}
        {
            Visibility_ = vis;
        }

        void Reset() {
            TDeprecatedCounter::operator=(0);
        }
    };

    struct THistogramCounter: public TCountableBase {
        explicit THistogramCounter(
            IHistogramCollectorPtr collector, bool derivative = true, EVisibility vis = EVisibility::Public)
            : Collector_(std::move(collector))
            , Derivative_(derivative)
        {
            Visibility_ = vis;
        }

        void Collect(i64 value) {
            Collector_->Collect(value);
        }

        void Collect(i64 value, ui64 count) {
            Collector_->Collect(value, count);
        }

        void Collect(double value, ui64 count) {
            Collector_->Collect(value, count);
        }

        void Collect(const IHistogramSnapshot& snapshot) {
            Collector_->Collect(snapshot);
        }

        void Accept(
            const std::string& labelName, const std::string& labelValue,
            ICountableConsumer& consumer) const override
        {
            if (IsVisible(Visibility(), consumer.Visibility())) {
                consumer.OnHistogram(labelName, labelValue, Collector_->Snapshot(), Derivative_);
            }
        }

        void Reset() {
            Collector_->Reset();
        }

        IHistogramSnapshotPtr Snapshot() const {
            return Collector_->Snapshot();
        }

    private:
        IHistogramCollectorPtr Collector_;
        bool Derivative_;
    };

    struct TExpiringHistogramCounter: public THistogramCounter {
        using THistogramCounter::THistogramCounter;
    };

    using THistogramPtr = TIntrusivePtr<THistogramCounter>;

#ifdef _MSC_VER
#pragma warning(pop)
#endif

    struct TDynamicCounters;

    typedef TIntrusivePtr<TDynamicCounters> TDynamicCounterPtr;
    struct TDynamicCounters: public TCountableBase {
    public:
        using TCounterPtr = TIntrusivePtr<TCounterForPtr>;
        using TOnLookupPtr = void (*)(const char *methodName, const std::string &name, const std::string &value);

    private:
        TRWMutex Lock;
        TCounterPtr LookupCounter; // Counts lookups by name
        TOnLookupPtr OnLookup = nullptr; // Called on each lookup if not nullptr, intended for lightweight tracing.

        typedef TIntrusivePtr<TCountableBase> TCountablePtr;

        struct TChildId {
            std::string LabelName;
            std::string LabelValue;
            TChildId() {
            }
            TChildId(const std::string& labelName, const std::string& labelValue)
                : LabelName(labelName)
                , LabelValue(labelValue)
            {
            }
            auto AsTuple() const {
                return std::make_tuple(std::cref(LabelName), std::cref(LabelValue));
            }
            friend bool operator <(const TChildId& x, const TChildId& y) {
                return x.AsTuple() < y.AsTuple();
            }
            friend bool operator ==(const TChildId& x, const TChildId& y) {
                return x.AsTuple() == y.AsTuple();
            }
            friend bool operator !=(const TChildId& x, const TChildId& y) {
                return x.AsTuple() != y.AsTuple();
            }
        };

        using TCounters = std::map<TChildId, TCountablePtr>;
        using TLabels = std::vector<TChildId>;

        /// XXX: hack for deferred removal of expired counters. Remove once Output* functions are not used for serialization
        mutable TCounters Counters;
        mutable std::atomic<int> ExpiringCount = 0;

    public:
        TDynamicCounters(TCountableBase::EVisibility visibility = TCountableBase::EVisibility::Public);

        TDynamicCounters(const TDynamicCounters *origin)
            : LookupCounter(origin->LookupCounter)
            , OnLookup(origin->OnLookup)
        {}

        ~TDynamicCounters() override;

        // This counter allows to track lookups by name within the whole subtree
        void SetLookupCounter(TCounterPtr lookupCounter) {
            TWriteGuard g(Lock);
            LookupCounter = lookupCounter;
        }

        void SetOnLookup(TOnLookupPtr onLookup) {
            TWriteGuard g(Lock);
            OnLookup = onLookup;
        }

        TWriteGuard LockForUpdate(const char *method, const std::string& name, const std::string& value) {
            auto res = TWriteGuard(Lock);
            if (LookupCounter) {
                ++*LookupCounter;
            }
            if (OnLookup) {
                OnLookup(method, name, value);
            }
            return res;
        }

        TStackVec<TCounters::value_type, 256> ReadSnapshot() const {
            RemoveExpired();
            TReadGuard g(Lock);
            TStackVec<TCounters::value_type, 256> items(Counters.begin(), Counters.end());
            return items;
        }

        TCounterPtr GetCounter(
            const std::string& value,
            bool derivative = false,
            TCountableBase::EVisibility visibility = TCountableBase::EVisibility::Public);

        TCounterPtr GetNamedCounter(
            const std::string& name,
            const std::string& value,
            bool derivative = false,
            TCountableBase::EVisibility visibility = TCountableBase::EVisibility::Public);

        THistogramPtr GetHistogram(
            const std::string& value,
            IHistogramCollectorPtr collector,
            bool derivative = true,
            TCountableBase::EVisibility visibility = TCountableBase::EVisibility::Public);

        THistogramPtr GetNamedHistogram(
            const std::string& name,
            const std::string& value,
            IHistogramCollectorPtr collector,
            bool derivative = true,
            TCountableBase::EVisibility visibility = TCountableBase::EVisibility::Public);

        // These counters will be automatically removed from the registry
        // when last reference to the counter expires.
        TCounterPtr GetExpiringCounter(
            const std::string& value,
            bool derivative = false,
            TCountableBase::EVisibility visibility = TCountableBase::EVisibility::Public);

        TCounterPtr GetExpiringNamedCounter(
            const std::string& name,
            const std::string& value,
            bool derivative = false,
            TCountableBase::EVisibility visibility = TCountableBase::EVisibility::Public);

        THistogramPtr GetExpiringHistogram(
            const std::string& value,
            IHistogramCollectorPtr collector,
            bool derivative = true,
            TCountableBase::EVisibility visibility = TCountableBase::EVisibility::Public);

        THistogramPtr GetExpiringNamedHistogram(
            const std::string& name,
            const std::string& value,
            IHistogramCollectorPtr collector,
            bool derivative = true,
            TCountableBase::EVisibility visibility = TCountableBase::EVisibility::Public);

        TCounterPtr FindCounter(const std::string& value) const;
        TCounterPtr FindNamedCounter(const std::string& name, const std::string& value) const;

        THistogramPtr FindHistogram(const std::string& value) const;
        THistogramPtr FindNamedHistogram(const std::string& name,const std::string& value) const;

        void RemoveCounter(const std::string &value);
        bool RemoveNamedCounter(const std::string& name, const std::string &value);
        void RemoveSubgroupChain(const std::vector<std::pair<std::string, std::string>>& chain);

        TIntrusivePtr<TDynamicCounters> GetSubgroup(const std::string& name, const std::string& value);
        TIntrusivePtr<TDynamicCounters> FindSubgroup(const std::string& name, const std::string& value) const;
        bool RemoveSubgroup(const std::string& name, const std::string& value);
        void ReplaceSubgroup(const std::string& name, const std::string& value, TIntrusivePtr<TDynamicCounters> subgroup);

        // Move all counters from specified subgroup and remove the subgroup.
        void MergeWithSubgroup(const std::string& name, const std::string& value);
        // Recursively reset all/deriv counters to 0.
        void ResetCounters(bool derivOnly = false);

        void RegisterSubgroup(const std::string& name,
            const std::string& value,
            TIntrusivePtr<TDynamicCounters> subgroup);

        void OutputHtml(IOutputStream& os) const;
        void EnumerateSubgroups(const std::function<void(const std::string& name, const std::string& value)>& output) const;

        // mostly for debugging purposes -- use accept with encoder instead
        void OutputPlainText(IOutputStream& os, const std::string& indent = "") const;

        void Accept(
            const std::string& labelName, const std::string& labelValue,
            ICountableConsumer& consumer) const override;

    private:
        TCounters Resign() {
            TCounters counters;
            TWriteGuard g(Lock);
            Counters.swap(counters);
            return counters;
        }

        void RegisterCountable(const std::string& name, const std::string& value, TCountablePtr countable);
        void RemoveExpired() const;

        template <bool expiring, class TCounterType, class... TArgs>
        TCountablePtr GetNamedCounterImpl(const std::string& name, const std::string& value, TArgs&&... args);

        template <class TCounterType>
        TCountablePtr FindNamedCounterImpl(const std::string& name, const std::string& value) const;
    };

}
