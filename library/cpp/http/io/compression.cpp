#include "compression.h"

#if defined(ENABLE_GPL)
#include <library/cpp/streams/lz/lz.h>
#endif

#include <library/cpp/string_builder/string_builder.h>
#include <library/cpp/string_utils/misc/misc.h>
#include <library/cpp/streams/brotli/brotli.h>
#include <library/cpp/streams/lzma/lzma.h>
#include <library/cpp/streams/bzip2/bzip2.h>

#include <library/cpp/blockcodecs/stream.h>
#include <library/cpp/blockcodecs/codecs.h>

#include <util/stream/zlib.h>


TCompressionCodecFactory::TCompressionCodecFactory() {
    auto gzip = [](auto s) {
        return std::make_unique<TZLibDecompress>(s);
    };

    Add("gzip", gzip, [](auto s) { return std::make_unique<TZLibCompress>(s, ZLib::GZip); });
    Add("deflate", gzip, [](auto s) { return std::make_unique<TZLibCompress>(s, ZLib::ZLib); });
    Add("br", [](auto s) { return std::make_unique<TBrotliDecompress>(s); }, [](auto s) { return std::make_unique<TBrotliCompress>(s, 4); });
    Add("x-gzip", gzip, [](auto s) { return std::make_unique<TZLibCompress>(s, ZLib::GZip); });
    Add("x-deflate", gzip, [](auto s) { return std::make_unique<TZLibCompress>(s, ZLib::ZLib); });

#if defined(ENABLE_GPL)
    const ui16 bs = 32 * 1024;

    Add("y-lzo", [](auto s) { return std::make_unique<TLzoDecompress>(s); }, [bs](auto s) { return std::make_unique<TLazy<TLzoCompress> >(s, bs); });
    Add("y-lzf", [](auto s) { return std::make_unique<TLzfDecompress>(s); }, [bs](auto s) { return std::make_unique<TLazy<TLzfCompress> >(s, bs); });
    Add("y-lzq", [](auto s) { return std::make_unique<TLzqDecompress>(s); }, [bs](auto s) { return std::make_unique<TLazy<TLzqCompress> >(s, bs); });
#endif

    Add("y-bzip2", [](auto s) { return std::make_unique<TBZipDecompress>(s); }, [](auto s) { return std::make_unique<TBZipCompress>(s); });
    Add("y-lzma", [](auto s) { return std::make_unique<TLzmaDecompress>(s); }, [](auto s) { return std::make_unique<TLzmaCompress>(s); });

    for (auto codecName : NBlockCodecs::ListAllCodecs()) {
        if (codecName.starts_with("zstd06")) {
            continue;
        }

        if (codecName.starts_with("zstd08")) {
            continue;
        }

        auto codec = NBlockCodecs::Codec(codecName);

        auto enc = [codec](auto s) {
            return std::make_unique<NBlockCodecs::TCodedOutput>(s, codec, 32 * 1024);
        };

        auto dec = [codec](auto s) {
            return std::make_unique<NBlockCodecs::TDecodedInput>(s, codec);
        };
        std::string fullName = NUtils::TYdbStringBuilder() << "z-" << codecName;
        Add(fullName, dec, enc);
    }
}

void TCompressionCodecFactory::Add(std::string_view name, TDecoderConstructor d, TEncoderConstructor e) {
    Strings_.emplace_back(name);
    Codecs_[Strings_.back()] = TCodec{d, e};
    BestCodecs_.emplace_back(Strings_.back());
}
