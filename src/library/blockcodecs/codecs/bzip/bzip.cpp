#include <src/library/blockcodecs/core/codecs.h>
#include <src/library/blockcodecs/core/common.h>
#include <src/library/blockcodecs/core/register.h>

#include <bzlib.h>

using namespace NBlockCodecs;

namespace {
    struct TBZipCodec: public TAddLengthCodec<TBZipCodec> {
        inline TBZipCodec(int level)
            : Level(level)
            , MyName("bzip2-" + std::to_string(Level))
        {
        }

        static inline size_t DoMaxCompressedLength(size_t in) noexcept {
            // very strange
            return in * 2 + 128;
        }

        std::string_view Name() const noexcept override {
            return MyName;
        }

        inline size_t DoCompress(const TData& in, void* buf) const {
            unsigned int ret = DoMaxCompressedLength(in.size());
            const int res = BZ2_bzBuffToBuffCompress((char*)buf, &ret, (char*)in.data(), in.size(), Level, 0, 0);
            if (res != BZ_OK) {
                ythrow TCompressError(res);
            }

            return ret;
        }

        inline void DoDecompress(const TData& in, void* out, size_t len) const {
            unsigned int tmp = SafeIntegerCast<unsigned int>(len);
            const int res = BZ2_bzBuffToBuffDecompress((char*)out, &tmp, (char*)in.data(), in.size(), 0, 0);

            if (res != BZ_OK) {
                ythrow TDecompressError(res);
            }

            if (len != tmp) {
                ythrow TDecompressError(len, tmp);
            }
        }

        const int Level;
        const std::string MyName;
    };

    struct TBZipRegistrar {
        TBZipRegistrar() {
            for (int i = 1; i < 10; ++i) {
                RegisterCodec(std::make_unique<TBZipCodec>(i));
            }
            RegisterAlias("bzip2", "bzip2-6");
        }
    };
    const TBZipRegistrar Registrar{};
}
