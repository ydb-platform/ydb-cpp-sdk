#pragma once

#include <src/util/generic/buffer.h>
#include <string_view>
#include <string>
#include <ydb-cpp-sdk/util/generic/typetraits.h>
#include <ydb-cpp-sdk/util/generic/yexception.h>

#include <memory>

namespace NBlockCodecs {
    struct TData: public std::string_view {
        inline TData() = default;

        Y_HAS_MEMBER(Data);
        Y_HAS_MEMBER(Size);

        template <class T, std::enable_if_t<!THasSize<T>::value || !THasData<T>::value, int> = 0>
        inline TData(const T& t)
            : std::string_view((const char*)t.data(), t.size())
        {
        }

        template <class T, std::enable_if_t<THasSize<T>::value && THasData<T>::value, int> = 0>
        inline TData(const T& t)
            : std::string_view((const char*)t.Data(), t.Size())
        {
        }
    };

    struct TCodecError: public yexception {
    };

    struct TNotFound: public TCodecError {
    };

    struct TDataError: public TCodecError {
    };

    struct ICodec {
        virtual ~ICodec();

        // main interface
        virtual size_t DecompressedLength(const TData& in) const = 0;
        virtual size_t MaxCompressedLength(const TData& in) const = 0;
        virtual size_t Compress(const TData& in, void* out) const = 0;
        virtual size_t Decompress(const TData& in, void* out) const = 0;

        virtual std::string_view Name() const noexcept = 0;

        // some useful helpers
        void Encode(const TData& in, TBuffer& out) const;
        void Decode(const TData& in, TBuffer& out) const;

        void Encode(const TData& in, std::string& out) const;
        void Decode(const TData& in, std::string& out) const;

        inline std::string Encode(const TData& in) const {
            std::string out;

            Encode(in, out);

            return out;
        }

        inline std::string Decode(const TData& in) const {
            std::string out;

            Decode(in, out);

            return out;
        }
    private:
        size_t GetDecompressedLength(const TData& in) const;
    };

    using TCodecPtr = std::unique_ptr<ICodec>;

    const ICodec* Codec(const std::string_view& name);

    // some aux methods
    typedef std::vector<std::string_view> TCodecList;
    TCodecList ListAllCodecs();
    std::string ListAllCodecsAsString();

    // SEARCH-8344: Get the size of max possible decompressed block
    size_t GetMaxPossibleDecompressedLength();
    // SEARCH-8344: Globally set the size of max possible decompressed block
    void SetMaxPossibleDecompressedLength(size_t maxPossibleDecompressedLength);

}
