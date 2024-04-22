#pragma once

#include "codecs.h"

#include <ydb-cpp-sdk/util/ysaveload.h>
#include <src/util/stream/null.h>
#include <ydb-cpp-sdk/util/stream/mem.h>
#include <ydb-cpp-sdk/util/string/cast.h>
#include <src/util/string/join.h>
#include <src/util/system/align.h>
#include <ydb-cpp-sdk/util/system/unaligned_mem.h>
#include <src/util/generic/hash.h>
#include <ydb-cpp-sdk/util/generic/cast.h>
#include <src/util/generic/buffer.h>
#include <ydb-cpp-sdk/util/generic/singleton.h>
#include <ydb-cpp-sdk/util/generic/algorithm.h>
#include <src/util/generic/mem_copy.h>

namespace NBlockCodecs {
    struct TDecompressError: public TDataError {
        TDecompressError(int code) {
            *this << "cannot decompress (errcode " << code << ")";
        }

        TDecompressError(size_t exp, size_t real) {
            *this << "broken input (expected len: " << exp << ", got: " << real << ")";
        }
    };

    struct TCompressError: public TDataError {
        TCompressError(int code) {
            *this << "cannot compress (errcode " << code << ")";
        }
    };

    struct TNullCodec: public ICodec {
        size_t DecompressedLength(const TData& in) const override {
            return in.size();
        }

        size_t MaxCompressedLength(const TData& in) const override {
            return in.size();
        }

        size_t Compress(const TData& in, void* out) const override {
            MemCopy((char*)out, in.data(), in.size());

            return in.size();
        }

        size_t Decompress(const TData& in, void* out) const override {
            MemCopy((char*)out, in.data(), in.size());

            return in.size();
        }

        std::string_view Name() const noexcept override {
            return std::string_view("null");
        }
    };

    template <class T>
    struct TAddLengthCodec: public ICodec {
        static inline void Check(const TData& in) {
            if (in.size() < sizeof(ui64)) {
                ythrow TDataError() << "too small input";
            }
        }

        size_t DecompressedLength(const TData& in) const override {
            Check(in);

            return ReadUnaligned<ui64>(in.data());
        }

        size_t MaxCompressedLength(const TData& in) const override {
            return T::DoMaxCompressedLength(in.size()) + sizeof(ui64);
        }

        size_t Compress(const TData& in, void* out) const override {
            ui64* ptr = (ui64*)out;

            WriteUnaligned<ui64>(ptr, (ui64) in.size());

            return Base()->DoCompress(in.empty() ? TData(std::string_view("")) : in, ptr + 1) + sizeof(*ptr);
        }

        size_t Decompress(const TData& in, void* out) const override {
            Check(in);

            const auto len = ReadUnaligned<ui64>(in.data());

            if (!len)
                return 0;
            TData inCopy(in);
            inCopy.remove_prefix(sizeof(len));
            Base()->DoDecompress(inCopy, out, len);
            return len;
        }

        inline const T* Base() const noexcept {
            return static_cast<const T*>(this);
        }
    };
}
