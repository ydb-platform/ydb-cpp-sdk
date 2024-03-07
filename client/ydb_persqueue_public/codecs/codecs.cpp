#include <library/cpp/streams/zstd/zstd.h>
#include <util/stream/buffer.h>
#include <util/stream/zlib.h>
#include <util/stream/mem.h>

#include "codecs.h"

namespace NYdb::NPersQueue {
namespace NCompressionDetails {

using TInputStreamVariant = std::variant<std::monostate, TZLibDecompress, TZstdDecompress>;

IInputStream* CreateDecompressorStream(TInputStreamVariant& inputStreamStorage, Ydb::PersQueue::V1::Codec codec, IInputStream* origin) {
    switch (codec) {
    case Ydb::PersQueue::V1::CODEC_GZIP:
        return &inputStreamStorage.emplace<TZLibDecompress>(origin);
    case Ydb::PersQueue::V1::CODEC_LZOP:
        throw yexception() << "LZO codec is disabled";
    case Ydb::PersQueue::V1::CODEC_ZSTD:
        return &inputStreamStorage.emplace<TZstdDecompress>(origin);
    default:
    //case Ydb::PersQueue::V1::CODEC_RAW:
    //case Ydb::PersQueue::V1::CODEC_UNSPECIFIED:
        throw yexception() << "unsupported codec value : " << ui64(codec);
    }
}

std::string Decompress(const Ydb::PersQueue::V1::MigrationStreamingReadServerMessage::DataBatch::MessageData& data) {
    TMemoryInput input(data.data().data(), data.data().size());
    std::string result;
    TStringOutput resultOutput(result);
    TInputStreamVariant inputStreamStorage;
    TransferData(CreateDecompressorStream(inputStreamStorage, data.codec(), &input), &resultOutput);
    return result;
}

class TZLibToStringCompressor: private TEmbedPolicy<TBufferOutput>, public TZLibCompress {
public:
    TZLibToStringCompressor(TBuffer& dst, ZLib::StreamType type, size_t quality)
        : TEmbedPolicy<TBufferOutput>(dst)
        , TZLibCompress(TEmbedPolicy::Ptr(), type, quality)
    {
    }
};

class TZstdToStringCompressor: private TEmbedPolicy<TBufferOutput>, public TZstdCompress {
public:
    TZstdToStringCompressor(TBuffer& dst, int quality)
        : TEmbedPolicy<TBufferOutput>(dst)
        , TZstdCompress(TEmbedPolicy::Ptr(), quality)
    {
    }
};

THolder<IOutputStream> CreateCoder(ECodec codec, TBuffer& result, int quality) {
    switch (codec) {
        case ECodec::GZIP:
            return MakeHolder<TZLibToStringCompressor>(result, ZLib::GZip, quality >= 0 ? quality : 6);
        case ECodec::LZOP:
            throw yexception() << "LZO codec is disabled";
        case ECodec::ZSTD:
            return MakeHolder<TZstdToStringCompressor>(result, quality);
        default:
            Y_ABORT("NOT IMPLEMENTED CODEC TYPE");
    }
}


} // namespace NDecompressionDetails

} // namespace NYdb::NPersQueue
