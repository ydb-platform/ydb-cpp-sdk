#pragma once

#include <src/library/streams/zstd/zstd.h>
#include <src/util/generic/buffer.h>
#include <src/util/stream/buffer.h>
#include <ydb-cpp-sdk/util/stream/mem.h>
#include <ydb-cpp-sdk/util/stream/str.h>
#include <src/util/stream/zlib.h>
#include <ydb-cpp-sdk/util/stream/output.h>
#include <ydb-cpp-sdk/util/system/spinlock.h>

#include <unordered_map>
#include <memory>

namespace NYdb::NTopic {

enum class ECodec : ui32 {
    RAW = 1,
    GZIP = 2,
    LZOP = 3,
    ZSTD = 4,
    CUSTOM = 10000,
};

inline const std::string& GetCodecId(const ECodec codec) {
    static std::unordered_map<ECodec, std::string> idByCodec{
        {ECodec::RAW, std::string(1, '\0')},
        {ECodec::GZIP, "\1"},
        {ECodec::LZOP, "\2"},
        {ECodec::ZSTD, "\3"}
    };
    Y_ABORT_UNLESS(idByCodec.contains(codec));
    return idByCodec[codec];
}

class ICodec {
public:
    virtual ~ICodec() = default;
    virtual std::string Decompress(const std::string& data) const = 0;
    virtual std::unique_ptr<IOutputStream> CreateCoder(TBuffer& result, int quality) const = 0;
};

class TGzipCodec final : public ICodec {
    class TZLibToStringCompressor: private TEmbedPolicy<TBufferOutput>, public TZLibCompress {
    public:
        TZLibToStringCompressor(TBuffer& dst, ZLib::StreamType type, size_t quality)
            : TEmbedPolicy<TBufferOutput>(dst)
            , TZLibCompress(TEmbedPolicy::Ptr(), type, quality)
        {
        }
    };

    std::string Decompress(const std::string& data) const override {
        TMemoryInput input(data.data(), data.size());
        std::string result;
        TStringOutput resultOutput(result);
        TZLibDecompress inputStreamStorage(&input);
        TransferData(&inputStreamStorage, &resultOutput);
        return result;
    }

    std::unique_ptr<IOutputStream> CreateCoder(TBuffer& result, int quality) const override {
        return std::make_unique<TZLibToStringCompressor>(result, ZLib::GZip, quality >= 0 ? quality : 6);
    }
};

class TZstdCodec final : public ICodec {
    class TZstdToStringCompressor: private TEmbedPolicy<TBufferOutput>, public TZstdCompress {
    public:
        TZstdToStringCompressor(TBuffer& dst, int quality)
            : TEmbedPolicy<TBufferOutput>(dst)
            , TZstdCompress(TEmbedPolicy::Ptr(), quality)
        {
        }
    };

    std::string Decompress(const std::string& data) const override {
        TMemoryInput input(data.data(), data.size());
        std::string result;
        TStringOutput resultOutput(result);
        TZstdDecompress inputStreamStorage(&input);
        TransferData(&inputStreamStorage, &resultOutput);
        return result;
    }

    std::unique_ptr<IOutputStream> CreateCoder(TBuffer& result, int quality) const override {
        return std::make_unique<TZstdToStringCompressor>(result, quality);
    }
};

class TUnsupportedCodec final : public ICodec {
    std::string Decompress(const std::string&) const override {
        throw yexception() << "use of unsupported codec";
    }

    std::unique_ptr<IOutputStream> CreateCoder(TBuffer&, int) const override {
        throw yexception() << "use of unsupported codec";
    }
};

class TCodecMap {
public:
    static TCodecMap& GetTheCodecMap() {
        static TCodecMap instance;
        return instance;
    }

    void Set(ui32 codecId, std::unique_ptr<ICodec>&& codecImpl) {
        with_lock(Lock) {
            Codecs[codecId] = std::move(codecImpl);
        }
    }

    const ICodec* GetOrThrow(ui32 codecId) const {
        with_lock(Lock) {
            if (!Codecs.contains(codecId)) {
                throw yexception() << "codec with id " << ui32(codecId) << " not provided";
            }
            return Codecs.at(codecId).get();
        }
    }


    TCodecMap(const TCodecMap&) = delete;
    TCodecMap(TCodecMap&&) = delete;
    TCodecMap& operator=(const TCodecMap&) = delete;
    TCodecMap& operator=(TCodecMap&&) = delete;

private:
    TCodecMap() = default;

private:
    std::unordered_map<ui32, std::unique_ptr<ICodec>> Codecs;
    TAdaptiveLock Lock;
};

} // namespace NYdb::NTopic