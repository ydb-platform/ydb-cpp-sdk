#pragma once

#include <util/digest/multi.h>
#include <util/digest/sequence.h>
#include <util/generic/algorithm.h>
#include <util/generic/maybe.h>
#include <util/generic/string.h>

#include <util/stream/output.h>
#include <util/string/builder.h>
#include <util/string/strip.h>

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
    template <typename std::stringBackend>
    class TLabelImpl: public ILabel {
    public:
        using std::stringType = std::stringBackend;

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

        inline const std::stringBackend& NameStr() const {
            return Name_;
        }

        inline const std::stringBackend& ValueStr() const {
            return Value_;
        }

        inline size_t Hash() const noexcept {
            return MultiHash(Name_, Value_);
        }

        std::stringBackend ToString() const {
            std::stringBackend buf = Name_;
            buf += '=';
            buf += Value_;

            return buf;
        }

        static TLabelImpl FromString(std::string_view str) {
            std::string_view name, value;
            Y_ENSURE(str.TrySplit('=', name, value),
                     "invalid label string format: '" << str << '\'');

            std::string_view nameStripped = StripString(name);
            Y_ENSURE(!nameStripped.empty(), "label name cannot be empty");

            std::string_view valueStripped = StripString(value);
            Y_ENSURE(!valueStripped.empty(), "label value cannot be empty");

            return {nameStripped, valueStripped};
        }

        static bool TryFromString(std::string_view str, TLabelImpl& label) {
            std::string_view name, value;
            if (!str.TrySplit('=', name, value)) {
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
        std::stringBackend Name_;
        std::stringBackend Value_;
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
    template <typename std::stringBackend>
    class TLabelsImpl: public ILabels {
    public:
        using value_type = TLabelImpl<std::stringBackend>;

        TLabelsImpl() = default;

        explicit TLabelsImpl(::NDetail::TReserveTag rt)
            : Labels_(std::move(rt))
        {}

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
            auto it = FindIf(Labels_, [name](const TLabelImpl<std::stringBackend>& label) {
                return name == std::string_view{label.Name()};
            });
            return it != Labels_.end();
        }

        bool Has(const std::string& name) const noexcept {
            auto it = FindIf(Labels_, [name](const TLabelImpl<std::stringBackend>& label) {
                return name == std::string_view{label.Name()};
            });
            return it != Labels_.end();
        }

        // XXX for backward compatibility
        TMaybe<TLabelImpl<std::stringBackend>> Find(std::string_view name) const {
            auto it = FindIf(Labels_, [name](const TLabelImpl<std::stringBackend>& label) {
                return name == std::string_view{label.Name()};
            });
            if (it == Labels_.end()) {
                return Nothing();
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

        TMaybe<TLabelImpl<std::stringBackend>> Extract(std::string_view name) {
            auto it = FindIf(Labels_, [name](const TLabelImpl<std::stringBackend>& label) {
                return name == std::string_view{label.Name()};
            });
            if (it == Labels_.end()) {
                return Nothing();
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

        TLabelImpl<std::stringBackend>& front() {
            return Labels_.front();
        }

        const TLabelImpl<std::stringBackend>& front() const {
            return Labels_.front();
        }

        TLabelImpl<std::stringBackend>& back() {
            return Labels_.back();
        }

        const TLabelImpl<std::stringBackend>& back() const {
            return Labels_.back();
        }

        TLabelImpl<std::stringBackend>& operator[](size_t index) {
            return Labels_[index];
        }

        const TLabelImpl<std::stringBackend>& operator[](size_t index) const {
            return Labels_[index];
        }

        TLabelImpl<std::stringBackend>& at(size_t index) {
            return Labels_.at(index);
        }

        const TLabelImpl<std::stringBackend>& at(size_t index) const {
            return Labels_.at(index);
        }

        size_t capacity() const {
            return Labels_.capacity();
        }

        TLabelImpl<std::stringBackend>* data() {
            return Labels_.data();
        }

        const TLabelImpl<std::stringBackend>* data() const {
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
        std::vector<TLabelImpl<std::stringBackend>>& AsVector() {
            return Labels_;
        }

        const std::vector<TLabelImpl<std::stringBackend>>& AsVector() const {
            return Labels_;
        }

    private:
        std::vector<TLabelImpl<std::stringBackend>> Labels_;
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

template<typename std::stringBackend>
struct THash<NMonitoring::TLabelsImpl<std::stringBackend>> {
    size_t operator()(const NMonitoring::TLabelsImpl<std::stringBackend>& labels) const noexcept {
        return labels.Hash();
    }
};

template <typename std::stringBackend>
struct THash<NMonitoring::TLabelImpl<std::stringBackend>> {
    inline size_t operator()(const NMonitoring::TLabelImpl<std::stringBackend>& label) const noexcept {
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
