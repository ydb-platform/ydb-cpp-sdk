#include "http_parser.h"

#include <library/cpp/blockcodecs/stream.h>
#include <library/cpp/blockcodecs/codecs.h>
#include <library/cpp/streams/brotli/brotli.h>

#include <util/generic/string.h>
#include <util/generic/yexception.h>
#include <util/stream/mem.h>
#include <util/stream/zlib.h>
#include <util/string/ascii.h>
#include <util/string/split.h>
#include <util/string/strip.h>

//#define DBGOUT(args) Cout << args << Endl;
#define DBGOUT(args)

namespace {
    const std::string BestCodings[] = {
        "gzip",
        "deflate",
        "br",
        "x-gzip",
        "x-deflate",
        "y-lzo",
        "y-lzf",
        "y-lzq",
        "y-bzip2",
        "y-lzma",
    };
}

std::string THttpParser::GetBestCompressionScheme() const {
    if (AcceptEncodings_.contains("*")) {
        return BestCodings[0];
    }

    for (auto& coding : BestCodings) {
        if (AcceptEncodings_.contains(coding)) {
            return coding;
        }
    }

    return std::string();
}

bool THttpParser::FirstLineParser() {
    if (Y_UNLIKELY(!ReadLine())) {
        return false;
    }

    CurrentLine_.swap(FirstLine_);

    try {
        std::string_view s(FirstLine_);
        if (MessageType_ == Response) {
            // Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
            std::string_view httpVersion, statusCode;
            GetNext(s, ' ', httpVersion);
            ParseHttpVersion(httpVersion);
            GetNext(s, ' ', statusCode);
            RetCode_ = FromString<unsigned>(statusCode);
        } else {
            // Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
            std::string_view httpVersion = s.After(' ').After(' ');
            ParseHttpVersion(httpVersion);
        }
    } catch (...) {
        throw THttpParseException() << "Cannot parse first line: " << CurrentExceptionMessage() << " First 80 chars of line: " << FirstLine_.substr(0, Min<size_t>(80ull, FirstLine_.size())).Quote();
    }

    return HeadersParser();
}

bool THttpParser::HeadersParser() {
    while (ReadLine()) {
        if (!CurrentLine_) {
            //end of headers
            DBGOUT("end of headers()");
            ParseHeaderLine();

            if (HasContentLength_) {
                if (ContentLength_ == 0) {
                    return OnEndParsing();
                }

                if (ContentLength_ < 1000000) {
                    Content_.reserve(ContentLength_ + 1);
                }
            }

            return !!ChunkInputState_ ? ChunkedContentParser() : ContentParser();
        }

        if (CurrentLine_[0] == ' ' || CurrentLine_[0] == '\t') {
            //continue previous header-line
            HeaderLine_ += CurrentLine_;
            CurrentLine_.remove(0);
        } else {
            ParseHeaderLine();
            HeaderLine_.swap(CurrentLine_);
        }
    }

    Parser_ = &THttpParser::HeadersParser;
    return false;
}

bool THttpParser::ContentParser() {
    DBGOUT("Content parsing()");
    if (HasContentLength_ && !BodyNotExpected_) {
        size_t rd = Min<size_t>(DataEnd_ - Data_, ContentLength_ - Content_.size());
        Content_.append(Data_, rd);
        Data_ += rd;
        DBGOUT("Content parsing: " << Content_.Size() << " from " << ContentLength_);
        if (Content_.size() == ContentLength_) {
            return OnEndParsing();
        }
    } else {
        if (MessageType_ == Request) {
            return OnEndParsing(); //RFC2616 4.4-5
        } else if (Y_UNLIKELY(BodyNotExpected_ || RetCode() < 200 || RetCode() == 204 || RetCode() == 304)) {
            return OnEndParsing(); //RFC2616 4.4-1
        }

        Content_.append(Data_, DataEnd_);
        Data_ = DataEnd_;
    }
    Parser_ = &THttpParser::ContentParser;
    return false;
}

bool THttpParser::ChunkedContentParser() {
    DBGOUT("ReadChunkedContent");
    TChunkInputState& ci = *ChunkInputState_;

    if (Content_.capacity() < static_cast<size_t>(DataEnd_ - Data_)) {
        //try reduce memory reallocations
        Content_.reserve(DataEnd_ - Data_);
    }

    do {
        if (!ci.LeftBytes_) {
            if (Y_UNLIKELY(!ReadLine())) { //read first chunk size or CRLF from prev chunk or CRLF from last chunk
                break;
            }

            if (Y_UNLIKELY(ci.ReadLastChunk_)) {
                return OnEndParsing();
            }

            if (!CurrentLine_) {
                // skip crlf from previous chunk
                if (!ReadLine()) {
                    break;
                }
            }
            Y_ENSURE(CurrentLine_.size(), "NEH: LeftBytes hex number cannot be empty. ");
            size_t size = CurrentLine_.find_first_of(" \t;");
            if (size == std::string::npos) {
                size = CurrentLine_.size();
            }
            ci.LeftBytes_ = IntFromString<ui32, 16, char>(CurrentLine_.c_str(), size);
            CurrentLine_.remove(0);
            if (!ci.LeftBytes_) { //detectect end of context marker - zero-size chunk, need read CRLF after empty chunk
                ci.ReadLastChunk_ = true;
                if (ReadLine()) {
                    return OnEndParsing();
                } else {
                    break;
                }
            }
        }

        size_t rd = Min<size_t>(DataEnd_ - Data_, ci.LeftBytes_);
        Content_.append(Data_, rd);
        Data_ += rd;
        ci.LeftBytes_ -= rd;
    } while (Data_ != DataEnd_);

    Parser_ = &THttpParser::ChunkedContentParser;
    return false;
}

bool THttpParser::OnEndParsing() {
    Parser_ = &THttpParser::OnEndParsing;
    ExtraDataSize_ = DataEnd_ - Data_;
    return true;
}

//continue read to CurrentLine_
bool THttpParser::ReadLine() {
    std::string_view in(Data_, DataEnd_);
    size_t endl = in.find('\n');

    if (Y_UNLIKELY(endl == std::string_view::npos)) {
        //input line not completed
        CurrentLine_.append(Data_, DataEnd_);
        return false;
    }

    CurrentLine_.append(in.data(), endl);
    if (Y_LIKELY(CurrentLine_.size())) {
        //remove '\r' from tail
        size_t withoutCR = CurrentLine_.size() - 1;
        if (CurrentLine_[withoutCR] == '\r') {
            CurrentLine_.remove(withoutCR);
        }
    }

    //Cout << "ReadLine:" << CurrentLine_ << Endl;
    Data_ += endl + 1;
    return true;
}

void THttpParser::ParseHttpVersion(std::string_view httpVersion) {
    if (!httpVersion.StartsWith("HTTP/", 5)) {
        throw yexception() << "expect 'HTTP/'";
    }
    httpVersion.Skip(5);
    {
        std::string_view major, minor;
        Split(httpVersion, '.', major, minor);
        HttpVersion_.Major = FromString<unsigned>(major);
        HttpVersion_.Minor = FromString<unsigned>(minor);
        if (Y_LIKELY(HttpVersion_.Major > 1 || HttpVersion_.Minor > 0)) {
            // since HTTP/1.1 Keep-Alive is default behaviour
            KeepAlive_ = true;
        }
    }
}

void THttpParser::ParseHeaderLine() {
    if (!!HeaderLine_) {
        if (CollectHeaders_) {
            THttpInputHeader hdr(HeaderLine_);

            Headers_.AddHeader(hdr);

            ApplyHeaderLine(hdr.Name(), hdr.Value());
        } else {
            //some dirty optimization (avoid reallocation new strings)
            size_t pos = HeaderLine_.find(':');

            if (pos == std::string::npos) {
                ythrow THttpParseException() << "can not parse http header(" << HeaderLine_.Quote() << ")";
            }

            std::string_view name(StripString(std::string_view(HeaderLine_.begin(), HeaderLine_.begin() + pos)));
            std::string_view val(StripString(std::string_view(HeaderLine_.begin() + pos + 1, HeaderLine_.end())));
            ApplyHeaderLine(name, val);
        }
        HeaderLine_.remove(0);
    }
}

void THttpParser::OnEof() {
    if (Parser_ == &THttpParser::ContentParser && !HasContentLength_ && !ChunkInputState_) {
        return; //end of content determined by end of input
    }
    throw THttpException() << std::string_view("incompleted http response");
}

bool THttpParser::DecodeContent() {
    if (!DecodeContent_) {
        return false;
    }

    if (!ContentEncoding_ || ContentEncoding_ == "identity" || ContentEncoding_ == "none") {
        DecodedContent_ = Content_;
        return false;
    }

    TMemoryInput in(Content_.data(), Content_.size());
    if (ContentEncoding_ == "gzip") {
        auto decompressor = TZLibDecompress(&in, ZLib::GZip);
        if (!GzipAllowMultipleStreams_) {
            decompressor.SetAllowMultipleStreams(false);
        }
        DecodedContent_ = decompressor.ReadAll();
    } else if (ContentEncoding_ == "deflate") {

        //https://tools.ietf.org/html/rfc1950
        bool definitelyNoZlibHeader;
        if (Content_.size() < 2) {
            definitelyNoZlibHeader = true;
        } else {
            const ui16 cmf = static_cast<ui8>(Content_[0]);
            const ui16 flg = static_cast<ui8>(Content_[1]);
            definitelyNoZlibHeader = ((cmf << 8) | flg) % 31 != 0;
        }

        try {
            DecodedContent_ = TZLibDecompress(&in, definitelyNoZlibHeader ? ZLib::Raw : ZLib::ZLib).ReadAll();
        }
        catch(...) {
            if (definitelyNoZlibHeader) {
                throw;
            }
            TMemoryInput retryInput(Content_.data(), Content_.size());
            DecodedContent_ = TZLibDecompress(&retryInput, ZLib::Raw).ReadAll();
        }
    } else if (ContentEncoding_.StartsWith("z-")) {
        // opposite for library/cpp/http/io/stream.h
        const NBlockCodecs::ICodec* codec = nullptr;
        try {
            const std::string_view codecName = std::string_view(ContentEncoding_).SubStr(2);
            if (codecName.StartsWith("zstd06") || codecName.StartsWith("zstd08")) {
                ythrow NBlockCodecs::TNotFound() << codecName;
            }
            codec = NBlockCodecs::Codec(codecName);
        } catch(const NBlockCodecs::TNotFound& exc) {
            throw THttpParseException() << "Unsupported content-encoding method: " << exc.AsStrBuf();
        }
        NBlockCodecs::TDecodedInput decoder(&in, codec);
        DecodedContent_ = decoder.ReadAll();
    } else if (ContentEncoding_ == "lz4") {
        const auto* codec = NBlockCodecs::Codec(std::string_view(ContentEncoding_));
        DecodedContent_ = codec->Decode(Content_);
    } else if (ContentEncoding_ == "br") {
        TBrotliDecompress decoder(&in);
        DecodedContent_ = decoder.ReadAll();
    } else {
        throw THttpParseException() << "Unsupported content-encoding method: " << ContentEncoding_;
    }
    return true;
}

void THttpParser::ApplyHeaderLine(const std::string_view& name, const std::string_view& val) {
    if (AsciiEqualsIgnoreCase(name, std::string_view("connection"))) {
        KeepAlive_ = AsciiEqualsIgnoreCase(val, std::string_view("keep-alive"));
    } else if (AsciiEqualsIgnoreCase(name, std::string_view("content-length"))) {
        Y_ENSURE(val.size(), "NEH: Content-Length cannot be empty string. ");
        ContentLength_ = FromString<ui64>(val);
        HasContentLength_ = true;
    } else if (AsciiEqualsIgnoreCase(name, std::string_view("transfer-encoding"))) {
        if (AsciiEqualsIgnoreCase(val, std::string_view("chunked"))) {
            ChunkInputState_ = new TChunkInputState();
        }
    } else if (AsciiEqualsIgnoreCase(name, std::string_view("accept-encoding"))) {
        std::string_view encodings(val);
        while (encodings.size()) {
            std::string_view enc = encodings.NextTok(',').After(' ').Before(' ');
            if (!enc) {
                continue;
            }
            std::string s(enc);
            s.to_lower();
            AcceptEncodings_.insert(s);
        }
    } else if (AsciiEqualsIgnoreCase(name, std::string_view("content-encoding"))) {
        std::string s(val);
        s.to_lower();
        ContentEncoding_ = s;
    }
}
