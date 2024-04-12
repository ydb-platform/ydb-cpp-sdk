#include <ydb-cpp-sdk/library/monlib/dynamic_counters/counters.h>

#include <src/library/monlib/service/pages/templates.h>
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/string/builder.h>

#include <ydb-cpp-sdk/util/generic/cast.h>
=======
#include <src/util/string/builder.h>

#include <src/util/generic/cast.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

using namespace NMonitoring;

namespace {
    TDynamicCounters* AsDynamicCounters(const TIntrusivePtr<TCountableBase>& ptr) {
        return dynamic_cast<TDynamicCounters*>(ptr.Get());
    }

    TCounterForPtr* AsCounter(const TIntrusivePtr<TCountableBase>& ptr) {
        return dynamic_cast<TCounterForPtr*>(ptr.Get());
    }

    TExpiringCounter* AsExpiringCounter(const TIntrusivePtr<TCountableBase>& ptr) {
        return dynamic_cast<TExpiringCounter*>(ptr.Get());
    }

    TExpiringHistogramCounter* AsExpiringHistogramCounter(const TIntrusivePtr<TCountableBase>& ptr) {
        return dynamic_cast<TExpiringHistogramCounter*>(ptr.Get());
    }

    THistogramCounter* AsHistogram(const TIntrusivePtr<TCountableBase>& ptr) {
        return dynamic_cast<THistogramCounter*>(ptr.Get());
    }

    TIntrusivePtr<TCounterForPtr> AsCounterRef(const TIntrusivePtr<TCountableBase>& ptr) {
        return VerifyDynamicCast<TCounterForPtr*>(ptr.Get());
    }

    TIntrusivePtr<TDynamicCounters> AsGroupRef(const TIntrusivePtr<TCountableBase>& ptr) {
        return VerifyDynamicCast<TDynamicCounters*>(ptr.Get());
    }

    THistogramPtr AsHistogramRef(const TIntrusivePtr<TCountableBase>& ptr) {
        return VerifyDynamicCast<THistogramCounter*>(ptr.Get());
    }

    bool IsExpiringCounter(const TIntrusivePtr<TCountableBase>& ptr) {
        return AsExpiringCounter(ptr) != nullptr || AsExpiringHistogramCounter(ptr) != nullptr;
    }
}

static constexpr std::string_view INDENT = "    ";

TDynamicCounters::TDynamicCounters(EVisibility vis)
{
    Visibility_ = vis;
}

TDynamicCounters::~TDynamicCounters() {
}

TDynamicCounters::TCounterPtr TDynamicCounters::GetExpiringCounter(const std::string& value, bool derivative, EVisibility vis) {
    return GetExpiringNamedCounter("sensor", value, derivative, vis);
}

TDynamicCounters::TCounterPtr TDynamicCounters::GetExpiringNamedCounter(const std::string& name, const std::string& value, bool derivative, EVisibility vis) {
    return AsCounterRef(GetNamedCounterImpl<true, TExpiringCounter>(name, value, derivative, vis));
}

TDynamicCounters::TCounterPtr TDynamicCounters::GetCounter(const std::string& value, bool derivative, EVisibility vis) {
    return GetNamedCounter("sensor", value, derivative, vis);
}

TDynamicCounters::TCounterPtr TDynamicCounters::GetNamedCounter(const std::string& name, const std::string& value, bool derivative, EVisibility vis) {
    return AsCounterRef(GetNamedCounterImpl<false, TCounterForPtr>(name, value, derivative, vis));
}

THistogramPtr TDynamicCounters::GetHistogram(const std::string& value, IHistogramCollectorPtr collector, bool derivative, EVisibility vis) {
    return GetNamedHistogram("sensor", value, std::move(collector), derivative, vis);
}

THistogramPtr TDynamicCounters::GetNamedHistogram(const std::string& name, const std::string& value, IHistogramCollectorPtr collector, bool derivative, EVisibility vis) {
    return AsHistogramRef(GetNamedCounterImpl<false, THistogramCounter>(name, value, std::move(collector), derivative, vis));
}

THistogramPtr TDynamicCounters::GetExpiringHistogram(const std::string& value, IHistogramCollectorPtr collector, bool derivative, EVisibility vis) {
    return GetExpiringNamedHistogram("sensor", value, std::move(collector), derivative, vis);
}

THistogramPtr TDynamicCounters::GetExpiringNamedHistogram(const std::string& name, const std::string& value, IHistogramCollectorPtr collector, bool derivative, EVisibility vis) {
    return AsHistogramRef(GetNamedCounterImpl<true, TExpiringHistogramCounter>(name, value, std::move(collector), derivative, vis));
}

TDynamicCounters::TCounterPtr TDynamicCounters::FindCounter(const std::string& value) const {
    return FindNamedCounter("sensor", value);
}

TDynamicCounters::TCounterPtr TDynamicCounters::FindNamedCounter(const std::string& name, const std::string& value) const {
    return AsCounterRef(FindNamedCounterImpl<TCounterForPtr>(name, value));
}

THistogramPtr TDynamicCounters::FindHistogram(const std::string& value) const {
    return FindNamedHistogram("sensor", value);
}

THistogramPtr TDynamicCounters::FindNamedHistogram(const std::string& name,const std::string& value) const {
    return AsHistogramRef(FindNamedCounterImpl<THistogramCounter>(name, value));
}

void TDynamicCounters::RemoveCounter(const std::string &value) {
    RemoveNamedCounter("sensor", value);
}

bool TDynamicCounters::RemoveNamedCounter(const std::string& name, const std::string &value) {
    auto g = LockForUpdate("RemoveNamedCounter", name, value);
    if (const auto it = Counters.find({name, value}); it != Counters.end() && AsCounter(it->second)) {
        Counters.erase(it);
    }
    return Counters.empty();
}

void TDynamicCounters::RemoveSubgroupChain(const std::vector<std::pair<std::string, std::string>>& chain) {
    std::vector<TIntrusivePtr<TDynamicCounters>> basePointers;
    basePointers.push_back(this);
    for (size_t i = 0; i < chain.size() - 1; ++i) {
        const auto& [name, value] = chain[i];
        auto& base = basePointers.back();
        basePointers.push_back(base->GetSubgroup(name, value));
        Y_ABORT_UNLESS(basePointers.back());
    }
    for (size_t i = chain.size(); i-- && basePointers[i]->RemoveSubgroup(chain[i].first, chain[i].second); ) {}
}

TIntrusivePtr<TDynamicCounters> TDynamicCounters::GetSubgroup(const std::string& name, const std::string& value) {
    auto res = FindSubgroup(name, value);
    if (!res) {
        auto g = LockForUpdate("GetSubgroup", name, value);
        const TChildId key(name, value);
        if (const auto it = Counters.lower_bound(key); it != Counters.end() && it->first == key) {
            res = AsGroupRef(it->second);
        } else {
            res = MakeIntrusive<TDynamicCounters>(this);
            Counters.emplace_hint(it, key, res);
        }
    }
    return res;
}

TIntrusivePtr<TDynamicCounters> TDynamicCounters::FindSubgroup(const std::string& name, const std::string& value) const {
    TReadGuard g(Lock);
    const auto it = Counters.find({name, value});
    return it != Counters.end() ? AsDynamicCounters(it->second) : nullptr;
}

bool TDynamicCounters::RemoveSubgroup(const std::string& name, const std::string& value) {
    auto g = LockForUpdate("RemoveSubgroup", name, value);
    if (const auto it = Counters.find({name, value}); it != Counters.end() && AsDynamicCounters(it->second)) {
        Counters.erase(it);
    }
    return Counters.empty();
}

void TDynamicCounters::ReplaceSubgroup(const std::string& name, const std::string& value, TIntrusivePtr<TDynamicCounters> subgroup) {
    auto g = LockForUpdate("ReplaceSubgroup", name, value);
    const auto it = Counters.find({name, value});
    Y_ABORT_UNLESS(it != Counters.end() && AsDynamicCounters(it->second));
    it->second = std::move(subgroup);
}

void TDynamicCounters::MergeWithSubgroup(const std::string& name, const std::string& value) {
    auto g = LockForUpdate("MergeWithSubgroup", name, value);
    auto it = Counters.find({name, value});
    Y_ABORT_UNLESS(it != Counters.end());
    TIntrusivePtr<TDynamicCounters> subgroup = AsDynamicCounters(it->second);
    Y_ABORT_UNLESS(subgroup);
    Counters.erase(it);
    Counters.merge(subgroup->Resign());
    AtomicAdd(ExpiringCount, AtomicSwap(&subgroup->ExpiringCount, 0));
}

void TDynamicCounters::ResetCounters(bool derivOnly) {
    TReadGuard g(Lock);
    for (auto& [key, value] : Counters) {
        if (auto counter = AsCounter(value)) {
            if (!derivOnly || counter->ForDerivative()) {
                *counter = 0;
            }
        } else if (auto subgroup = AsDynamicCounters(value)) {
            subgroup->ResetCounters(derivOnly);
        }
    }
}

void TDynamicCounters::RegisterCountable(const std::string& name, const std::string& value, TCountablePtr countable) {
    Y_ABORT_UNLESS(countable);
    auto g = LockForUpdate("RegisterCountable", name, value);
    const bool inserted = Counters.emplace(TChildId(name, value), std::move(countable)).second;
    Y_ABORT_UNLESS(inserted);
}

void TDynamicCounters::RegisterSubgroup(const std::string& name, const std::string& value, TIntrusivePtr<TDynamicCounters> subgroup) {
    RegisterCountable(name, value, subgroup);
}

void TDynamicCounters::OutputHtml(IOutputStream& os) const {
    HTML(os) {
        PRE() {
            OutputPlainText(os);
        }
    }
}

void TDynamicCounters::EnumerateSubgroups(const std::function<void(const std::string& name, const std::string& value)>& output) const {
    TReadGuard g(Lock);
    for (const auto& [key, value] : Counters) {
        if (AsDynamicCounters(value)) {
            output(key.LabelName, key.LabelValue);
        }
    }
}

void TDynamicCounters::OutputPlainText(IOutputStream& os, const std::string& indent) const {
    auto snap = ReadSnapshot();
    // mark private records in plain text output
    auto outputVisibilityMarker = [] (EVisibility vis) {
        return vis == EVisibility::Private ? "\t[PRIVATE]" : "";
    };

    for (const auto& [key, value] : snap) {
        if (const auto counter = AsCounter(value)) {
            os << indent
               << key.LabelName << '=' << key.LabelValue
               << ": " << counter->Val()
               << outputVisibilityMarker(counter->Visibility())
               << '\n';
        } else if (const auto histogram = AsHistogram(value)) {
            os << indent
               << key.LabelName << '=' << key.LabelValue
               << ":"
               << outputVisibilityMarker(histogram->Visibility())
               << "\n";

            auto snapshot = histogram->Snapshot();
            for (ui32 i = 0, count = snapshot->Count(); i < count; i++) {
                os << indent << INDENT << std::string_view("bin=");
                TBucketBound bound = snapshot->UpperBound(i);
                if (bound == Max<TBucketBound>()) {
                    os << std::string_view("inf");
                } else {
                   os << bound;
                }
                os << ": " << snapshot->Value(i) << '\n';
            }
        }
    }

    for (const auto& [key, value] : snap) {
        if (const auto subgroup = AsDynamicCounters(value)) {
            os << "\n";
            os << indent << key.LabelName << "=" << key.LabelValue << ":\n";
            subgroup->OutputPlainText(os, TStringBuilder() << indent << INDENT);
        }
    }
}

void TDynamicCounters::Accept(const std::string& labelName, const std::string& labelValue, ICountableConsumer& consumer) const {
    if (!IsVisible(Visibility(), consumer.Visibility())) {
        return;
    }

    consumer.OnGroupBegin(labelName, labelValue, this);
    for (auto& [key, value] : ReadSnapshot()) {
        value->Accept(key.LabelName, key.LabelValue, consumer);
    }
    consumer.OnGroupEnd(labelName, labelValue, this);
}

void TDynamicCounters::RemoveExpired() const {
    if (AtomicGet(ExpiringCount) == 0) {
        return;
    }

    TWriteGuard g(Lock);
    TAtomicBase count = 0;

    for (auto it = Counters.begin(); it != Counters.end();) {
        if (IsExpiringCounter(it->second) && it->second->RefCount() == 1) {
            it = Counters.erase(it);
            ++count;
        } else {
            ++it;
        }
    }

    AtomicSub(ExpiringCount, count);
}

template <bool expiring, class TCounterType, class... TArgs>
TDynamicCounters::TCountablePtr TDynamicCounters::GetNamedCounterImpl(const std::string& name, const std::string& value, TArgs&&... args) {
    {
        TReadGuard g(Lock);
        auto it = Counters.find({name, value});
        if (it != Counters.end()) {
            return it->second;
        }
    }

    auto g = LockForUpdate("GetNamedCounterImpl", name, value);
    const TChildId key(name, value);
    auto it = Counters.lower_bound(key);
    if (it == Counters.end() || it->first != key) {
        auto value = MakeIntrusive<TCounterType>(std::forward<TArgs>(args)...);
        it = Counters.emplace_hint(it, key, value);
        if constexpr (expiring) {
            AtomicIncrement(ExpiringCount);
        }
    }
    return it->second;
}

template <class TCounterType>
TDynamicCounters::TCountablePtr TDynamicCounters::FindNamedCounterImpl(const std::string& name, const std::string& value) const {
    TReadGuard g(Lock);
    auto it = Counters.find({name, value});
    return it != Counters.end() ? it->second : nullptr;
}

