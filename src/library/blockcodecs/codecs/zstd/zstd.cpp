#include <src/library/blockcodecs/core/codecs.h>
#include <src/library/blockcodecs/core/common.h>
#include <src/library/blockcodecs/core/register.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/string/builder.h>
=======
#include <src/util/string/builder.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/string/builder.h>
>>>>>>> origin/main

#define ZSTD_STATIC_LINKING_ONLY
#include <zstd.h>

using namespace NBlockCodecs;

namespace {
    struct TZStd08Codec: public TAddLengthCodec<TZStd08Codec> {
        inline TZStd08Codec(unsigned level)
            : Level(level)
            , MyName(TStringBuilder() << std::string_view("zstd08_") << std::to_string(Level))
        {
        }

        static inline size_t CheckError(size_t ret, const char* what) {
            if (ZSTD_isError(ret)) {
                ythrow yexception() << what << std::string_view(" zstd error: ") << ZSTD_getErrorName(ret);
            }

            return ret;
        }

        static inline size_t DoMaxCompressedLength(size_t l) noexcept {
            return ZSTD_compressBound(l);
        }

        inline size_t DoCompress(const TData& in, void* out) const {
            return CheckError(ZSTD_compress(out, DoMaxCompressedLength(in.size()), in.data(), in.size(), Level), "compress");
        }

        inline void DoDecompress(const TData& in, void* out, size_t dsize) const {
            const size_t res = CheckError(ZSTD_decompress(out, dsize, in.data(), in.size()), "decompress");

            if (res != dsize) {
                ythrow TDecompressError(dsize, res);
            }
        }

        std::string_view Name() const noexcept override {
            return MyName;
        }

        const unsigned Level;
        const std::string MyName;
    };

    struct TZStd08Registrar {
        TZStd08Registrar() {
            for (int i = 1; i <= ZSTD_maxCLevel(); ++i) {
                RegisterCodec(MakeHolder<TZStd08Codec>(i));
                RegisterAlias("zstd_" + std::to_string(i), "zstd08_" + std::to_string(i));
            }
        }
    };
    const TZStd08Registrar Registrar{};
}
