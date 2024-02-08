#pragma once

#include "stream.h"

#include <util/generic/deque.h>
#include <util/generic/hash.h>

class TCompressionCodecFactory {
public:
    using TDecoderConstructor = std::function<THolder<IInputStream>(IInputStream*)>;
    using TEncoderConstructor = std::function<THolder<IOutputStream>(IOutputStream*)>;

    TCompressionCodecFactory();

    static inline TCompressionCodecFactory& Instance() noexcept {
        return *SingletonWithPriority<TCompressionCodecFactory, 0>();
    }

    inline const TDecoderConstructor* FindDecoder(std::string_view name) const {
        if (auto codec = Codecs_.FindPtr(name)) {
            return &codec->Decoder;
        }

        return nullptr;
    }

    inline const TEncoderConstructor* FindEncoder(std::string_view name) const {
        if (auto codec = Codecs_.FindPtr(name)) {
            return &codec->Encoder;
        }

        return nullptr;
    }

    inline TArrayRef<const std::string_view> GetBestCodecs() const {
        return BestCodecs_;
    }

private:
    void Add(std::string_view name, TDecoderConstructor d, TEncoderConstructor e);

    struct TCodec {
        TDecoderConstructor Decoder;
        TEncoderConstructor Encoder;
    };

    TDeque<std::string> Strings_;
    THashMap<std::string_view, TCodec> Codecs_;
    std::vector<std::string_view> BestCodecs_;
};

namespace NHttp {
    template <typename F>
    std::string ChooseBestCompressionScheme(F accepted, TArrayRef<const std::string_view> available) {
        if (available.empty()) {
            return "identity";
        }

        if (accepted("*")) {
            return std::string(available[0]);
        }

        for (const auto& coding : available) {
            std::string s(coding);
            if (accepted(s)) {
                return s;
            }
        }

        return "identity";
    }
}
