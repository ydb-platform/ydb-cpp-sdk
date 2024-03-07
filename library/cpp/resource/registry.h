#pragma once

#include <string>
#include <string_view>

#include "resource.h"

namespace NResource {
    std::string Compress(const std::string_view data);
    std::string Decompress(const std::string_view data);

    class IMatch {
    public:
        virtual void OnMatch(const TResource& res) = 0;
        virtual ~IMatch() = default;
    };

    class IStore {
    public:
        virtual void Store(const std::string_view key, const std::string_view data) = 0;
        virtual bool Has(const std::string_view key) const = 0;
        virtual bool FindExact(const std::string_view key, std::string* out) const = 0;
        virtual void FindMatch(const std::string_view subkey, IMatch& cb) const = 0;
        virtual size_t Count() const noexcept = 0;
        virtual std::string_view KeyByIndex(size_t idx) const = 0;
        virtual ~IStore() = default;
    };

    IStore* CommonStore();

    struct TRegHelper {
        inline TRegHelper(const std::string_view key, const std::string_view data) {
            CommonStore()->Store(key, data);
        }
    };
}
