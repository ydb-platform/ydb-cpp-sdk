#include <ydb-cpp-sdk/library/resource/resource.h>

#include "registry.h"

#include <src/util/generic/yexception.h>
#include <ydb-cpp-sdk/util/generic/xrange.h>

using namespace NResource;

bool NResource::FindExact(const std::string_view key, std::string* out) {
    return CommonStore()->FindExact(key, out);
}

void NResource::FindMatch(const std::string_view subkey, TResources* out) {
    struct TMatch: public IMatch {
        inline TMatch(TResources* r)
            : R(r)
        {
        }

        void OnMatch(const TResource& res) override {
            R->push_back(res);
        }

        TResources* R;
    };

    TMatch m(out);

    CommonStore()->FindMatch(subkey, m);
}

bool NResource::Has(const std::string_view key) {
    return CommonStore()->Has(key);
}

std::string NResource::Find(const std::string_view key) {
    std::string ret;

    if (FindExact(key, &ret)) {
        return ret;
    }

    ythrow yexception() << "can not find resource with path " << key;
}

size_t NResource::Count() noexcept {
    return CommonStore()->Count();
}

std::string_view NResource::KeyByIndex(size_t idx) {
    return CommonStore()->KeyByIndex(idx);
}

std::vector<std::string_view> NResource::ListAllKeys() {
    std::vector<std::string_view> res;
    res.reserve(NResource::Count());
    for (auto i : xrange(NResource::Count())) {
        res.push_back(NResource::KeyByIndex(i));
    }
    return res;
}
