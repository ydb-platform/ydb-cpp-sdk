#include <ydb-cpp-sdk/library/http/io/stream.h>

#include <src/library/string_utils/stream/stream.h>
<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/string/strip.h>
#include <ydb-cpp-sdk/util/string/escape.h>
=======
#include <src/util/string/strip.h>
#include <src/util/string/escape.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/string/strip.h>
#include <ydb-cpp-sdk/util/string/escape.h>
>>>>>>> origin/main

static inline std::string_view Trim(const char* b, const char* e) noexcept {
    return StripString(std::string_view(b, e));
}

static inline bool HeaderNameEqual(std::string_view headerName, std::string_view expectedName) noexcept {
    // Most headers names have distinct sizes.
    // Size comparison adds small overhead if all headers have the same size (~4% or lower with size = 4),
    // but significantly speeds up the case where sizes are different (~4.5x for expectedName.size() = 4 and headerName.size() = 5)
    return headerName.size() == expectedName.size() && AsciiCompareIgnoreCase(headerName, expectedName) == 0;
}

THttpInputHeader::THttpInputHeader(const std::string_view header) {
    size_t pos = header.find(':');

    if (pos == std::string::npos) {
        ythrow THttpParseException() << "can not parse http header(" << NUtils::Quote(header) << ")";
    }

    Name_ = std::string(header.cbegin(), header.cbegin() + pos);
    Value_ = ::ToString(Trim(header.cbegin() + pos + 1, header.cend()));
}

THttpInputHeader::THttpInputHeader(std::string name, std::string value)
    : Name_(std::move(name))
    , Value_(std::move(value))
{
}

void THttpInputHeader::OutTo(IOutputStream* stream) const {
    typedef IOutputStream::TPart TPart;

    const TPart parts[] = {
        TPart(Name_),
        TPart(": ", 2),
        TPart(Value_),
        TPart::CrLf(),
    };

    stream->Write(parts, sizeof(parts) / sizeof(*parts));
}

THttpHeaders::THttpHeaders(IInputStream* stream) {
    std::string header;
    std::string line;

    bool rdOk = NUtils::ReadLine(*stream, header);
    while (rdOk && !header.empty()) {
        rdOk = NUtils::ReadLine(*stream, line);

        if (rdOk && ((line[0] == ' ') || (line[0] == '\t'))) {
            header += line;
        } else {
            AddHeader(THttpInputHeader(header));
            header = line;
        }
    }
}

bool THttpHeaders::HasHeader(const std::string_view header) const {
    return FindHeader(header);
}

const THttpInputHeader* THttpHeaders::FindHeader(const std::string_view header) const {
    for (const auto& hdr : Headers_) {
        if (HeaderNameEqual(hdr.Name(), header)) {
            return &hdr;
        }
    }
    return nullptr;
}

void THttpHeaders::RemoveHeader(const std::string_view header) {
    for (auto h = Headers_.begin(); h != Headers_.end(); ++h) {
        if (HeaderNameEqual(h->Name(), header)) {
            Headers_.erase(h);
            return;
        }
    }
}

void THttpHeaders::AddOrReplaceHeader(const THttpInputHeader& header) {
    std::string_view name = header.Name();
    for (auto& hdr : Headers_) {
        if (HeaderNameEqual(hdr.Name(), name)) {
            hdr = header;
            return;
        }
    }

    AddHeader(header);
}

void THttpHeaders::AddHeader(THttpInputHeader header) {
    Headers_.push_back(std::move(header));
}

void THttpHeaders::OutTo(IOutputStream* stream) const {
    for (TConstIterator header = Begin(); header != End(); ++header) {
        header->OutTo(stream);
    }
}

template <>
void Out<THttpHeaders>(IOutputStream& out, const THttpHeaders& h) {
    h.OutTo(&out);
}
