#pragma once

#include <ydb-cpp-sdk/util/generic/noncopyable.h>
#include <ydb-cpp-sdk/util/generic/yexception.h>
#include <ydb-cpp-sdk/util/system/types.h>

#include <string_view>
#include <vector>
#include <unordered_map>

namespace NMonitoring {
    ////////////////////////////////////////////////////////////////////////////////
    // TStringPoolBuilder
    ////////////////////////////////////////////////////////////////////////////////
    class TStringPoolBuilder {
    public:
        struct TValue: TNonCopyable {
            TValue(ui32 idx, ui32 freq)
                : Index{idx}
                , Frequency{freq}
            {
            }

            ui32 Index;
            ui32 Frequency;
        };

    public:
        const TValue* PutIfAbsent(std::string_view str);
        const TValue* GetByIndex(ui32 index) const;

        /// Determines whether pool must be sorted by value frequencies
        TStringPoolBuilder& SetSorted(bool sorted) {
            RequiresSorting_ = sorted;
            return *this;
        }

        TStringPoolBuilder& Build();

        std::string_view Get(ui32 index) const {
            if (RequiresSorting_) {
                Y_ENSURE(IsBuilt_, "Pool must be sorted first");
            }

            return StrVector_.at(index).first;
        }

        std::string_view Get(const TValue* value) const {
            return StrVector_.at(value->Index).first;
        }

        template <typename TConsumer>
        void ForEach(TConsumer&& c) {
            Y_ENSURE(IsBuilt_, "Pool must be built first");

            for (const auto& value : StrVector_) {
                c(value.first, value.second->Index, value.second->Frequency);
            }
        }

        size_t BytesSize() const noexcept {
            return BytesSize_;
        }

        size_t Count() const noexcept {
            return StrMap_.size();
        }

    private:
        std::unordered_map<std::string, TValue> StrMap_;
        std::vector<std::pair<std::string_view, TValue*>> StrVector_;
        bool RequiresSorting_ = false;
        bool IsBuilt_ = false;
        size_t BytesSize_ = 0;
    };

    ////////////////////////////////////////////////////////////////////////////////
    // TStringPool
    ////////////////////////////////////////////////////////////////////////////////
    class TStringPool {
    public:
        TStringPool(const char* data, ui32 size) {
            InitIndex(data, size);
        }

        std::string_view Get(ui32 i) const {
            return Index_.at(i);
        }

        size_t Size() const {
            return Index_.size();
        }

        size_t SizeBytes() const {
            return Index_.capacity() * sizeof(std::string_view);
        }

    private:
        void InitIndex(const char* data, ui32 size);

    private:
        std::vector<std::string_view> Index_;
    };

} // namespace NMonitoring
