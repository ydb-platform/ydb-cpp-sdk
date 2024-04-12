#include "compression.h"

#if defined(ENABLE_GPL)
#include <src/library/streams/lz/lz.h>
#endif

#include <src/util/string/builder.h>
#include <src/library/string_utils/misc/misc.h>
#include <src/library/streams/brotli/brotli.h>
#include <src/library/streams/lzma/lzma.h>
#include <src/library/streams/bzip2/bzip2.h>

#include <src/library/blockcodecs/stream.h>
#include <src/library/blockcodecs/codecs.h>

#include <src/util/stream/zlib.h>


TCompressionCodecFactory::TCompressionCodecFactory() {
    auto gzip = [](auto s) {
        return MakeHolder<TZLibDecompress>(s);
    };

    Add("gzip", gzip, [](auto s) { return MakeHolder<TZLibCompress>(s, ZLib::GZip); });
    Add("deflate", gzip, [](auto s) { return MakeHolder<TZLibCompress>(s, ZLib::ZLib); });
    Add("br", [](auto s) { return MakeHolder<TBrotliDecompress>(s); }, [](auto s) { return MakeHolder<TBrotliCompress>(s, 4); });
    Add("x-gzip", gzip, [](auto s) { return MakeHolder<TZLibCompress>(s, ZLib::GZip); });
    Add("x-deflate", gzip, [](auto s) { return MakeHolder<TZLibCompress>(s, ZLib::ZLib); });

#if defined(ENABLE_GPL)
    const ui16 bs = 32 * 1024;

    Add("y-lzo", [](auto s) { return MakeHolder<TLzoDecompress>(s); }, [bs](auto s) { return MakeHolder<TLazy<TLzoCompress> >(s, bs); });
    Add("y-lzf", [](auto s) { return MakeHolder<TLzfDecompress>(s); }, [bs](auto s) { return MakeHolder<TLazy<TLzfCompress> >(s, bs); });
    Add("y-lzq", [](auto s) { return MakeHolder<TLzqDecompress>(s); }, [bs](auto s) { return MakeHolder<TLazy<TLzqCompress> >(s, bs); });
#endif

    Add("y-bzip2", [](auto s) { return MakeHolder<TBZipDecompress>(s); }, [](auto s) { return MakeHolder<TBZipCompress>(s); });
    Add("y-lzma", [](auto s) { return MakeHolder<TLzmaDecompress>(s); }, [](auto s) { return MakeHolder<TLzmaCompress>(s); });

    for (auto codecName : NBlockCodecs::ListAllCodecs()) {
        if (codecName.starts_with("zstd08")) {
            continue;
        }

        auto codec = NBlockCodecs::Codec(codecName);

        auto enc = [codec](auto s) {
            return MakeHolder<NBlockCodecs::TCodedOutput>(s, codec, 32 * 1024);
        };

        auto dec = [codec](auto s) {
            return MakeHolder<NBlockCodecs::TDecodedInput>(s, codec);
        };
        std::string fullName = TStringBuilder() << "z-" << codecName;
        Add(fullName, dec, enc);
    }
}

void TCompressionCodecFactory::Add(std::string_view name, TDecoderConstructor d, TEncoderConstructor e) {
    Strings_.emplace_back(name);
    Codecs_[Strings_.back()] = TCodec{d, e};
    BestCodecs_.emplace_back(Strings_.back());
}
