#pragma once

#include <ydb-cpp-sdk/library/string_utils/misc/misc.h>

#include <src/util/digest/multi.h>
#include <src/util/digest/sequence.h>
#include <src/util/generic/algorithm.h>

#include <src/util/stream/output.h>
#include <src/util/string/builder.h>
#include <src/util/string/strip.h>

#include <optional>
#include <type_traits>

namespace NMonitoring {
    struct ILabel {
        virtual ~ILabel() = default;

        virtual std::string_view Name() const noexcept = 0;
        virtual std::string_view Value() const noexcept = 0;
    };

    ///////////////////////////////////////////////////////////////////////////
    // TLabel
    ///////////////////////////////////////////////////////////////////////////
    template <typename TStringBackend>
    class TLabelImpl: public ILabel {
    public:
        using TStringType = TStringBackend;

        TLabelImpl() = default;

        inline TLabelImpl(std::string_view name, std::string_view value)
            : Name_{name}
            , Value_{value}
        {
        }

        inline bool operator==(const TLabelImpl& rhs) const noexcept {
            return Name_ == rhs.Name_ && Value_ == rhs.Value_;
        }

        inline bool operator!=(const TLabelImpl& rhs) const noexcept {
            return !(*this == rhs);
        }

        inline std::string_view Name() const noexcept {
            return Name_;
        }

        inline std::string_view Value() const noexcept {
            return Value_;
        }

        inline const TStringBackend& NameStr() const {
            return Name_;
        }

        inline const TStringBackend& ValueStr() const {
            return Value_;
        }

        inline size_t Hash() const noexcept {
            return MultiHash(Name_, Value_);
        }

        TStringBackend ToString() const {
            TStringBackend buf = Name_;
            buf += '=';
            buf += Value_;

            return buf;
        }

        static TLabelImpl FromString(std::string_view str) {
            std::string_view name, value;
            Y_ENSURE(NUtils::TrySplit(str, name, value, '='),
                     "invalid label string format: '" << str << '\'');

            std::string_view nameStripped = StripString(name);
            Y_ENSURE(!nameStripped.empty(), "label name cannot be empty");

            std::string_view valueStripped = StripString(value);
            Y_ENSURE(!valueStripped.empty(), "label value cannot be empty");

            return {nameStripped, valueStripped};
        }

        static bool TryFromString(std::string_view str, TLabelImpl& label) {
            std::string_view name, value;
            if (!NUtils::TrySplit(str, name, value, '=')) {
                return false;
            }

            std::string_view nameStripped = StripString(name);
            if (nameStripped.empty()) {
                return false;
            }

            std::string_view valueStripped = StripString(value);
            if (valueStripped.empty()) {
                return false;
            }

            label = {nameStripped, valueStripped};
            return true;
        }

    private:
        TStringBackend Name_;
        TStringBackend Value_;
    };

    using TLabel = TLabelImpl<std::string>;

    struct ILabels : public TThrRefBase {
        struct TIterator {
            TIterator() = default;
            TIterator(const ILabels* labels, size_t idx = 0)
                : Labels_{labels}
                , Idx_{idx}
            {
            }

            TIterator& operator++() noexcept {
                Idx_++;
                return *this;
            }

            void operator+=(size_t i) noexcept {
                Idx_ += i;
            }

            bool operator==(const TIterator& other) const noexcept {
                return Idx_ == other.Idx_;
            }

            bool operator!=(const TIterator& other) const noexcept {
                return !(*this == other);
            }

            const ILabel* operator->() const noexcept {
                Y_DEBUG_ABORT_UNLESS(Labels_);
                return Labels_->Get(Idx_);
            }

            const ILabel& operator*() const noexcept {
                Y_DEBUG_ABORT_UNLESS(Labels_);
                return *Labels_->Get(Idx_);
            }


        private:
            const ILabels* Labels_{nullptr};
            size_t Idx_{0};
        };

        virtual ~ILabels() = default;

        virtual bool Add(std::string_view name, std::string_view value) noexcept = 0;
        virtual bool Add(const ILabel& label) noexcept {
            return Add(label.Name(), label.Value());
        }

        virtual bool Has(std::string_view name) const noexcept = 0;

        virtual size_t Size() const noexcept = 0;
        virtual bool Empty() const noexcept = 0;
        virtual void Clear() noexcept = 0;

        virtual size_t Hash() const noexcept = 0;

        virtual std::optional<const ILabel*> Get(std::string_view name) const = 0;

        // NB: there's no guarantee that indices are preserved after any object modification
        virtual const ILabel* Get(size_t idx) const = 0;

        TIterator begin() const {
            return TIterator{this};
        }

        TIterator end() const {
            return TIterator{this, Size()};
        }
    };

    bool TryLoadLabelsFromString(std::string_view sb, ILabels& labels);
    bool TryLoadLabelsFromString(IInputStream& is, ILabels& labels);

    ///////////////////////////////////////////////////////////////////////////
    // TLabels
    ///////////////////////////////////////////////////////////////////////////
    template <typename TStringBackend>
    class TLabelsImpl: public ILabels {
    public:
        using value_type = TLabelImpl<TStringBackend>;

        TLabelsImpl() = default;

        explicit TLabelsImpl(size_t count)
            : Labels_(count)
        {}

        explicit TLabelsImpl(size_t count, const value_type& label)
            : Labels_(count, label)
        {}

        TLabelsImpl(std::initializer_list<value_type> il)
            : Labels_(std::move(il))
        {}

        TLabelsImpl(const TLabelsImpl&) = default;
        TLabelsImpl& operator=(const TLabelsImpl&) = default;

        TLabelsImpl(TLabelsImpl&&) noexcept = default;
        TLabelsImpl& operator=(TLabelsImpl&&) noexcept = default;

        inline bool operator==(const TLabelsImpl& rhs) const {
            return Labels_ == rhs.Labels_;
        }

        inline bool operator!=(const TLabelsImpl& rhs) const {
            return Labels_ != rhs.Labels_;
        }

        bool Add(std::string_view name, std::string_view value) noexcept override {
            if (Has(name)) {
                return false;
            }

            Labels_.emplace_back(name, value);
            return true;
        }

        using ILabels::Add;

        bool Has(std::string_view name) const noexcept override {
            auto it = FindIf(Labels_, [name](const TLabelImpl<TStringBackend>& label) {
                return name == std::string_view{label.Name()};
            });
            return it != Labels_.end();
        }

        bool Has(const std::string& name) const noexcept {
            auto it = FindIf(Labels_, [name](const TLabelImpl<TStringBackend>& label) {
                return name == std::string_view{label.Name()};
            });
            return it != Labels_.end();
        }

        // XXX for backward compatibility
        std::optional<TLabelImpl<TStringBackend>> Find(std::string_view name) const {
            auto it = FindIf(Labels_, [name](const TLabelImpl<TStringBackend>& label) {
                return name == std::string_view{label.Name()};
            });
            if (it == Labels_.end()) {
                return std::nullopt;
            }
            return *it;
        }

        std::optional<const ILabel*> Get(std::string_view name) const override {
            auto it = FindIf(Labels_, [name] (auto&& l) {
                return name == l.Name();
            });

            if (it == Labels_.end()) {
                return {};
            }

            return &*it;
        }

        const ILabel* Get(size_t idx) const noexcept override {
            return &(*this)[idx];
        }

        std::optional<TLabelImpl<TStringBackend>> Extract(std::string_view name) {
            auto it = FindIf(Labels_, [name](const TLabelImpl<TStringBackend>& label) {
                return name == std::string_view{label.Name()};
            });
            if (it == Labels_.end()) {
                return std::nullopt;
            }
            TLabel tmp = *it;
            Labels_.erase(it);
            return tmp;
        }

        void SortByName() {
            std::sort(Labels_.begin(), Labels_.end(), [](const auto& lhs, const auto& rhs) {
                return lhs.Name() < rhs.Name();
            });
        }

        inline size_t Hash() const noexcept override {
            return TSimpleRangeHash()(Labels_);
        }

        inline TLabel* Data() const noexcept {
            return const_cast<TLabel*>(Labels_.data());
        }

        inline size_t Size() const noexcept override {
            return Labels_.size();
        }

        inline bool Empty() const noexcept override {
            return Labels_.empty();
        }

        inline void Clear() noexcept override {
            Labels_.clear();
        }

        TLabelImpl<TStringBackend>& front() {
            return Labels_.front();
        }

        const TLabelImpl<TStringBackend>& front() const {
            return Labels_.front();
        }

        TLabelImpl<TStringBackend>& back() {
            return Labels_.back();
        }

        const TLabelImpl<TStringBackend>& back() const {
            return Labels_.back();
        }

        TLabelImpl<TStringBackend>& operator[](size_t index) {
            return Labels_[index];
        }

        const TLabelImpl<TStringBackend>& operator[](size_t index) const {
            return Labels_[index];
        }

        TLabelImpl<TStringBackend>& at(size_t index) {
            return Labels_.at(index);
        }

        const TLabelImpl<TStringBackend>& at(size_t index) const {
            return Labels_.at(index);
        }

        size_t capacity() const {
            return Labels_.capacity();
        }

        TLabelImpl<TStringBackend>* data() {
            return Labels_.data();
        }

        const TLabelImpl<TStringBackend>* data() const {
            return Labels_.data();
        }

        size_t size() const {
            return Labels_.size();
        }

        bool empty() const {
            return Labels_.empty();
        }

        void clear() {
            Labels_.clear();
        }

        using ILabels::begin;
        using ILabels::end;

        using iterator = ILabels::TIterator;
        using const_iterator = iterator;

    protected:
        std::vector<TLabelImpl<TStringBackend>>& AsVector() {
            return Labels_;
        }

        const std::vector<TLabelImpl<TStringBackend>>& AsVector() const {
            return Labels_;
        }

    private:
        std::vector<TLabelImpl<TStringBackend>> Labels_;
    };

    using TLabels = TLabelsImpl<std::string>;
    using ILabelsPtr = TIntrusivePtr<ILabels>;

    template <typename T>
    ILabelsPtr MakeLabels() {
        return MakeIntrusive<TLabelsImpl<T>>();
    }

    template <typename T>
    ILabelsPtr MakeLabels(std::initializer_list<TLabelImpl<T>> labels) {
        return MakeIntrusive<TLabelsImpl<T>>(labels);
    }

    inline ILabelsPtr MakeLabels(TLabels&& labels) {
        return MakeIntrusive<TLabels>(std::move(labels));
    }
}

template<>
struct THash<NMonitoring::ILabelsPtr> {
    size_t operator()(const NMonitoring::ILabelsPtr& labels) const noexcept {
        return labels->Hash();
    }

    size_t operator()(const NMonitoring::ILabels& labels) const noexcept {
        return labels.Hash();
    }
};

template<typename TStringBackend>
struct THash<NMonitoring::TLabelsImpl<TStringBackend>> {
    size_t operator()(const NMonitoring::TLabelsImpl<TStringBackend>& labels) const noexcept {
        return labels.Hash();
    }
};

template <typename TStringBackend>
struct THash<NMonitoring::TLabelImpl<TStringBackend>> {
    inline size_t operator()(const NMonitoring::TLabelImpl<TStringBackend>& label) const noexcept {
        return label.Hash();
    }
};

inline bool operator==(const NMonitoring::ILabels& lhs, const NMonitoring::ILabels& rhs) {
    if (lhs.Size() != rhs.Size()) {
        return false;
    }

    for (auto&& l : lhs) {
        auto rl = rhs.Get(l.Name());
        if (!rl || (*rl)->Value() != l.Value()) {
            return false;
        }
    }

    return true;
}

bool operator==(const NMonitoring::ILabelsPtr& lhs, const NMonitoring::ILabelsPtr& rhs) = delete;
bool operator==(const NMonitoring::ILabels& lhs, const NMonitoring::ILabelsPtr& rhs) = delete;
bool operator==(const NMonitoring::ILabelsPtr& lhs, const NMonitoring::ILabels& rhs) = delete;

template<>
struct TEqualTo<NMonitoring::ILabelsPtr> {
    bool operator()(const NMonitoring::ILabelsPtr& lhs, const NMonitoring::ILabelsPtr& rhs) {
        return *lhs == *rhs;
    }

    bool operator()(const NMonitoring::ILabelsPtr& lhs, const NMonitoring::ILabels& rhs) {
        return *lhs == rhs;
    }

    bool operator()(const NMonitoring::ILabels& lhs, const NMonitoring::ILabelsPtr& rhs) {
        return lhs == *rhs;
    }
};

#define Y_MONLIB_DEFINE_LABELS_OUT(T) \
template <> \
void Out<T>(IOutputStream& out, const T& labels) { \
    Out<NMonitoring::ILabels>(out, labels); \
}

#define Y_MONLIB_DEFINE_LABEL_OUT(T) \
template <> \
void Out<T>(IOutputStream& out, const T& label) { \
    Out<NMonitoring::ILabel>(out, label); \
}
