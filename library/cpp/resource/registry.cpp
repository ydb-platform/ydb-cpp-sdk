#include "registry.h"

#include <library/cpp/blockcodecs/core/codecs.h>
#include <library/cpp/string_utils/misc/misc.h>

#include <util/system/yassert.h>
#include <util/generic/hash.h>
#include <util/generic/deque.h>
#include <util/generic/singleton.h>
#include <util/string/escape.h>

#include <iostream>

using namespace NResource;
using namespace NBlockCodecs;

namespace {
    inline const ICodec* GetCodec() noexcept {
        static const ICodec* ret = Codec("zstd08_5");

        return ret;
    }

    typedef std::pair<std::string_view, std::string_view> TDescriptor;

    struct TStore: public IStore, public THashMap<std::string_view, TDescriptor*> {
        void Store(const std::string_view key, const std::string_view data) override {
            if (contains(key)) {
                const std::string_view value = (*this)[key]->second;
                if (value != data) {
                    size_t vsize = GetCodec()->DecompressedLength(value);
                    size_t dsize = GetCodec()->DecompressedLength(data);
                    if (vsize + dsize < 1000) {
                        Y_ABORT_UNLESS(false, "Redefinition of key %s:\n"
                                 "  old value: %s,\n"
                                 "  new value: %s.",
                                 NUtils::Quote(key).c_str(),
                                 NUtils::Quote(Decompress(value)).c_str(),
                                 NUtils::Quote(Decompress(data)).c_str());
                    } else {
                        Y_ABORT_UNLESS(false, "Redefinition of key %s,"
                                 " old size: %zu,"
                                 " new size: %zu.",
                                 NUtils::Quote(key).c_str(), vsize, dsize);
                    }
                }
            } else {
                D_.push_back(TDescriptor(key, data));
                (*this)[key] = &D_.back();
            }

            Y_ABORT_UNLESS(size() == Count(), "size mismatch");
        }

        bool Has(const std::string_view key) const override {
            return contains(key);
        }

        bool FindExact(const std::string_view key, std::string* out) const override {
            if (TDescriptor* const* res = FindPtr(key)) {
                // temporary
                // https://st.yandex-team.ru/DEVTOOLS-3985
                try {
                    *out = Decompress((*res)->second);
                } catch (const yexception& e) {
                    if (!std::string{std::getenv("RESOURCE_DECOMPRESS_DIAG") ? std::getenv("RESOURCE_DECOMPRESS_DIAG") : ""}.empty()) {
                        std::cerr << "Can't decompress resource " << key << std::endl << e.what() << std::endl;
                    }
                    throw e;
                }

                return true;
            }

            return false;
        }

        void FindMatch(const std::string_view subkey, IMatch& cb) const override {
            for (const auto& it : *this) {
                if (it.first.starts_with(subkey)) {
                    // temporary
                    // https://st.yandex-team.ru/DEVTOOLS-3985
                    try {
                        const TResource res = {
                            it.first, Decompress(it.second->second)};
                        cb.OnMatch(res);
                    } catch (const yexception& e) {
                        if (!std::string{getenv("RESOURCE_DECOMPRESS_DIAG")}.empty()) {
                            std::cerr << "Can't decompress resource " << it.first << std::endl << e.what() << std::endl;
                        }
                        throw e;
                    }
                }
            }
        }

        size_t Count() const noexcept override {
            return D_.size();
        }

        std::string_view KeyByIndex(size_t idx) const override {
            return D_.at(idx).first;
        }

        typedef std::deque<TDescriptor> TDescriptors;
        TDescriptors D_;
    };
}

std::string NResource::Compress(const std::string_view data) {
    return GetCodec()->Encode(data);
}

std::string NResource::Decompress(const std::string_view data) {
    return GetCodec()->Decode(data);
}

IStore* NResource::CommonStore() {
    return SingletonWithPriority<TStore, 0>();
}
