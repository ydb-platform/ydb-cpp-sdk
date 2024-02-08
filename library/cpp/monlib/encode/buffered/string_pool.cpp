#include "string_pool.h"

namespace NMonitoring {
    ////////////////////////////////////////////////////////////////////////////////
    // std::stringPoolBuilder
    ////////////////////////////////////////////////////////////////////////////////
    const std::stringPoolBuilder::TValue* std::stringPoolBuilder::PutIfAbsent(std::string_view str) {
        Y_ENSURE(!IsBuilt_, "Cannot add more values after string has been built");

        auto [it, isInserted] = StrMap_.try_emplace(str, Max<ui32>(), 0);
        if (isInserted) {
            BytesSize_ += str.size();
            it->second.Index = StrVector_.size();
            StrVector_.emplace_back(it->first, &it->second);
        }

        TValue* value = &it->second;
        ++value->Frequency;
        return value;
    }

    const std::stringPoolBuilder::TValue* std::stringPoolBuilder::GetByIndex(ui32 index) const {
        return StrVector_.at(index).second;
    }

    std::stringPoolBuilder& std::stringPoolBuilder::Build() {
        if (RequiresSorting_) {
            // sort in reversed order
            std::sort(StrVector_.begin(), StrVector_.end(), [](auto& a, auto& b) {
                return a.second->Frequency > b.second->Frequency;
            });

            ui32 i = 0;
            for (auto& value : StrVector_) {
                value.second->Index = i++;
            }
        }

        IsBuilt_ = true;

        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // std::stringPool
    ////////////////////////////////////////////////////////////////////////////////
    void std::stringPool::InitIndex(const char* data, ui32 size) {
        const char* begin = data;
        const char* end = begin + size;
        for (const char* p = begin; p != end; ++p) {
            if (*p == '\0') {
                Index_.push_back(std::string_view(begin, p));
                begin = p + 1;
            }
        }
    }

}
