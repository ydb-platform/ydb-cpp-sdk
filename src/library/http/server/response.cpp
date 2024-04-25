#include <ydb-cpp-sdk/library/http/server/response.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/stream/mem.h>
=======
#include <src/util/stream/mem.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/stream/mem.h>
>>>>>>> origin/main

THttpResponse& THttpResponse::AddMultipleHeaders(const THttpHeaders& headers) {
    for (THttpHeaders::TConstIterator i = headers.Begin(); i != headers.End(); ++i) {
        this->Headers.AddHeader(*i);
    }
    return *this;
}

THttpResponse& THttpResponse::SetContentType(const std::string_view& contentType) {
    Headers.AddOrReplaceHeader(THttpInputHeader("Content-Type", ToString(contentType)));

    return *this;
}

void THttpResponse::OutTo(IOutputStream& os) const {
    std::vector<IOutputStream::TPart> parts;
    const size_t FIRST_LINE_PARTS = 3;
    const size_t HEADERS_PARTS = Headers.Count() * 4;
    const size_t CONTENT_PARTS = 5;
    parts.reserve(FIRST_LINE_PARTS + HEADERS_PARTS + CONTENT_PARTS);

    // first line
    parts.push_back(IOutputStream::TPart(std::string_view("HTTP/1.1 ")));
    parts.push_back(IOutputStream::TPart(HttpCodeStrEx(Code)));
    parts.push_back(IOutputStream::TPart::CrLf());

    // headers
    for (THttpHeaders::TConstIterator i = Headers.Begin(); i != Headers.End(); ++i) {
        parts.push_back(IOutputStream::TPart(i->Name()));
        parts.push_back(IOutputStream::TPart(std::string_view(": ")));
        parts.push_back(IOutputStream::TPart(i->Value()));
        parts.push_back(IOutputStream::TPart::CrLf());
    }

    char buf[50];

    if (!Content.empty()) {
        TMemoryOutput mo(buf, sizeof(buf));

        mo << Content.size();

        parts.push_back(IOutputStream::TPart(std::string_view("Content-Length: ")));
        parts.push_back(IOutputStream::TPart(buf, mo.Buf() - buf));
        parts.push_back(IOutputStream::TPart::CrLf());
    }

    // content
    parts.push_back(IOutputStream::TPart::CrLf());

    if (!Content.empty()) {
        parts.push_back(IOutputStream::TPart(Content));
    }

    os.Write(parts.data(), parts.size());
}

template <>
void Out<THttpResponse>(IOutputStream& os, const THttpResponse& resp) {
    resp.OutTo(os);
}
