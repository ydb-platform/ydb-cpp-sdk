#include <src/library/blockcodecs/core/codecs.h>
#include <src/library/blockcodecs/core/common.h>
#include <src/library/blockcodecs/core/register.h>

<<<<<<< HEAD
#include <ydb-cpp-sdk/util/string/builder.h>
=======
#include <src/util/string/builder.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))

#include <brotli/encode.h>
#include <brotli/decode.h>

#include <string_view>

using namespace NBlockCodecs;

namespace {
    struct TBrotliCodec : public TAddLengthCodec<TBrotliCodec> {
        static constexpr int BEST_QUALITY = 11;

        inline TBrotliCodec(ui32 level)
            : Quality(level)
            , MyName(TStringBuilder() << std::string_view("brotli_") << std::to_string(level))
        {
        }

        static inline size_t DoMaxCompressedLength(size_t l) noexcept {
            return BrotliEncoderMaxCompressedSize(l);
        }

        inline size_t DoCompress(const TData& in, void* out) const {
            size_t resultSize = MaxCompressedLength(in);
            auto result = BrotliEncoderCompress(
                                /*quality*/ Quality,
                                /*window*/ BROTLI_DEFAULT_WINDOW,
                                /*mode*/ BrotliEncoderMode::BROTLI_MODE_GENERIC,
                                /*input_size*/ in.size(),
                                /*input_buffer*/ (const unsigned char*)(in.data()),
                                /*encoded_size*/ &resultSize,
                                /*encoded_buffer*/ static_cast<unsigned char*>(out));
            if (result != BROTLI_TRUE) {
                ythrow yexception() << "internal brotli error during compression";
            }

            return resultSize;
        }

        inline void DoDecompress(const TData& in, void* out, size_t dsize) const {
            size_t decoded = dsize;
            auto result = BrotliDecoderDecompress(in.size(), (const unsigned char*)in.data(), &decoded, static_cast<unsigned char*>(out));
            if (result != BROTLI_DECODER_RESULT_SUCCESS) {
                ythrow yexception() << "internal brotli error during decompression";
            } else if (decoded != dsize) {
                ythrow TDecompressError(dsize, decoded);
            }
        }

        std::string_view Name() const noexcept override {
            return MyName;
        }

        const int Quality = BEST_QUALITY;
        const std::string MyName;
    };

    struct TBrotliRegistrar {
        TBrotliRegistrar() {
            for (int i = 1; i <= TBrotliCodec::BEST_QUALITY; ++i) {
                RegisterCodec(MakeHolder<TBrotliCodec>(i));
            }
        }
    };
    const TBrotliRegistrar Registrar{};
}
