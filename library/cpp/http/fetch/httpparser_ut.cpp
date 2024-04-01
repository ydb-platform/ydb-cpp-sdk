#include "httpparser.h"

#include <library/cpp/testing/unittest/registar.h>

#define ENUM_OUT(arg)  \
    case type ::arg: { \
        out << #arg;   \
        return;        \
    }

template <>
void Out<THttpParserBase::States>(IOutputStream& out, THttpParserBase::States st) {
    using type = THttpParserBase::States;
    switch (st) {
        ENUM_OUT(hp_error)
        ENUM_OUT(hp_eof)
        ENUM_OUT(hp_in_header)
        ENUM_OUT(hp_read_alive)
        ENUM_OUT(hp_read_closed)
        ENUM_OUT(hp_begin_chunk_header)
        ENUM_OUT(hp_chunk_header)
        ENUM_OUT(hp_read_chunk)
    }
}

namespace {
    class TSomethingLikeFakeCheck;

    using TTestHttpParser = THttpParser<TSomethingLikeFakeCheck>;

    class TSomethingLikeFakeCheck {
        std::string Body_;

    public:
        const std::string& Body() const {
            return Body_;
        }

        // other functions are not really called by THttpParser
        void CheckDocPart(const void* buf, size_t len, THttpHeader* /* header */) {
            std::string s(static_cast<const char*>(buf), len);
            std::cout << "State = " << static_cast<TTestHttpParser*>(this)->GetState() << ", CheckDocPart(" << s.Quote() << ")\n";
            Body_ += s;
        }
    };

}

Y_UNIT_TEST_SUITE(TestHttpParser) {
    Y_UNIT_TEST(TestTrivialRequest) {
        const std::string blob{
            "GET /search?q=hi HTTP/1.1\r\n"
            "Host:  www.google.ru:8080 \r\n"
            "\r\n"};
        THttpHeader hdr;
        THttpParser<> parser;
        parser.Init(&hdr);
        parser.Parse((void*)blob.data(), blob.size());
        UNIT_ASSERT_EQUAL(parser.GetState(), parser.hp_error); // can't parse request as response
    }

    // XXX: `entity_size` is i32 and `content_length` is i64!
    Y_UNIT_TEST(TestTrivialResponse) {
        const std::string blob{
            "HTTP/1.1 200 Ok\r\n"
            "Content-Length: 2\r\n"
            "\r\n"
            "OK"};
        THttpHeader hdr;
        TTestHttpParser parser;
        parser.Init(&hdr);
        parser.Parse((void*)blob.data(), blob.size());
        UNIT_ASSERT_EQUAL(parser.GetState(), parser.hp_eof);
        UNIT_ASSERT_EQUAL(parser.Body(), "OK");
        UNIT_ASSERT_EQUAL(hdr.header_size, strlen(
                                               "HTTP/1.1 200 Ok\r\n"
                                               "Content-Length: 2\r\n"
                                               "\r\n"));
        UNIT_ASSERT_EQUAL(hdr.entity_size, strlen("OK"));
    }

    // XXX: `entity_size` is off by one in TE:chunked case.
    Y_UNIT_TEST(TestChunkedResponse) {
        const std::string blob{
            "HTTP/1.1 200 OK\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n"
            "2\r\n"
            "Ok\r\n"
            "8\r\n"
            "AllRight\r\n"
            "0\r\n"
            "\r\n"};
        THttpHeader hdr;
        TTestHttpParser parser;
        parser.Init(&hdr);
        parser.Parse((void*)blob.data(), blob.size());
        UNIT_ASSERT_EQUAL(parser.GetState(), parser.hp_eof);
        UNIT_ASSERT_EQUAL(parser.Body(), "OkAllRight");
        UNIT_ASSERT_EQUAL(hdr.header_size, strlen(
                                               "HTTP/1.1 200 OK\r\n"
                                               "Transfer-Encoding: chunked\r\n"
                                               "\r\n"));
        const int off_by_one_err = -1; // XXX: it really looks so
        UNIT_ASSERT_EQUAL(hdr.entity_size + off_by_one_err, strlen(
                                                                "2\r\n"
                                                                "Ok\r\n"
                                                                "8\r\n"
                                                                "AllRight\r\n"
                                                                "0\r\n"
                                                                "\r\n"));
    }

    static const std::string PipelineClenBlob_{
        "HTTP/1.1 200 Ok\r\n"
        "Content-Length: 4\r\n"
        "\r\n"
        "OK\r\n"
        "HTTP/1.1 200 Zz\r\n"
        "Content-Length: 4\r\n"
        "\r\n"
        "ZZ\r\n"};

    void AssertPipelineClen(TTestHttpParser & parser, const THttpHeader& hdr) {
        UNIT_ASSERT_EQUAL(parser.GetState(), parser.hp_eof);
        UNIT_ASSERT_EQUAL(4, hdr.content_length);
        UNIT_ASSERT_EQUAL(hdr.header_size, strlen(
                                               "HTTP/1.1 200 Ok\r\n"
                                               "Content-Length: 4\r\n"
                                               "\r\n"));
    }

    Y_UNIT_TEST(TestPipelineClenByteByByte) {
        const std::string& blob = PipelineClenBlob_;
        THttpHeader hdr;
        TTestHttpParser parser;
        parser.Init(&hdr);
        for (size_t i = 0; i < blob.size(); ++i) {
            const std::string_view d{blob, i, 1};
            parser.Parse((void*)d.data(), d.size());
            std::cout << std::string(d).Quote() << " -> " << parser.GetState() << std::endl;
        }
        AssertPipelineClen(parser, hdr);
        UNIT_ASSERT_EQUAL(parser.Body(), "OK\r\n");
        UNIT_ASSERT_EQUAL(hdr.entity_size, hdr.content_length);
    }

    // XXX: Content-Length is ignored, Body() looks unexpected!
    Y_UNIT_TEST(TestPipelineClenOneChunk) {
        const std::string& blob = PipelineClenBlob_;
        THttpHeader hdr;
        TTestHttpParser parser;
        parser.Init(&hdr);
        parser.Parse((void*)blob.data(), blob.size());
        AssertPipelineClen(parser, hdr);
        UNIT_ASSERT_EQUAL(parser.Body(),
                          "OK\r\n"
                          "HTTP/1.1 200 Zz\r\n"
                          "Content-Length: 4\r\n"
                          "\r\n"
                          "ZZ\r\n");
        UNIT_ASSERT_EQUAL(hdr.entity_size, strlen(
                                               "OK\r\n"
                                               "HTTP/1.1 200 Zz\r\n"
                                               "Content-Length: 4\r\n"
                                               "\r\n"
                                               "ZZ\r\n"));
    }

    static const std::string PipelineChunkedBlob_{
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n"
        "2\r\n"
        "Ok\r\n"
        "8\r\n"
        "AllRight\r\n"
        "0\r\n"
        "\r\n"
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n"
        "2\r\n"
        "Yo\r\n"
        "8\r\n"
        "uWin!Iam\r\n"
        "0\r\n"
        "\r\n"};

    void AssertPipelineChunked(TTestHttpParser & parser, const THttpHeader& hdr) {
        UNIT_ASSERT_EQUAL(parser.GetState(), parser.hp_eof);
        UNIT_ASSERT_EQUAL(parser.Body(), "OkAllRight");
        UNIT_ASSERT_EQUAL(-1, hdr.content_length);
        UNIT_ASSERT_EQUAL(hdr.header_size, strlen(
                                               "HTTP/1.1 200 OK\r\n"
                                               "Transfer-Encoding: chunked\r\n"
                                               "\r\n"));
        const int off_by_one_err = -1;
        UNIT_ASSERT_EQUAL(hdr.entity_size + off_by_one_err, strlen(
                                                                "2\r\n"
                                                                "Ok\r\n"
                                                                "8\r\n"
                                                                "AllRight\r\n"
                                                                "0\r\n"
                                                                "\r\n"));
    }

    Y_UNIT_TEST(TestPipelineChunkedByteByByte) {
        const std::string& blob = PipelineChunkedBlob_;
        THttpHeader hdr;
        TTestHttpParser parser;
        parser.Init(&hdr);
        for (size_t i = 0; i < blob.size(); ++i) {
            const std::string_view d{blob, i, 1};
            parser.Parse((void*)d.data(), d.size());
            std::cout << std::string(d).Quote() << " -> " << parser.GetState() << std::endl;
            if (blob.size() / 2 - 1 <= i) // last \n sets EOF
                UNIT_ASSERT_EQUAL(parser.GetState(), parser.hp_eof);
        }
        AssertPipelineChunked(parser, hdr);
    }

    Y_UNIT_TEST(TestPipelineChunkedOneChunk) {
        const std::string& blob = PipelineChunkedBlob_;
        THttpHeader hdr;
        TTestHttpParser parser;
        parser.Init(&hdr);
        parser.Parse((void*)blob.data(), blob.size());
        AssertPipelineChunked(parser, hdr);
    }
}
