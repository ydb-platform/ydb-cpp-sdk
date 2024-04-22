#include <ydb-cpp-sdk/library/http/io/stream.h>

#include "compression.h"
#include "chunk.h"

#include <ydb-cpp-sdk/library/string_utils/misc/misc.h>
#include <src/library/string_utils/stream/stream.h>

#include <src/util/stream/buffered.h>
#include <src/util/stream/length.h>
#include <src/util/stream/multi.h>
#include <src/util/stream/null.h>
#include <src/util/stream/tee.h>

#include <ydb-cpp-sdk/util/system/yassert.h>

#include <ydb-cpp-sdk/util/network/socket.h>

#include <ydb-cpp-sdk/util/string/strip.h>

#include <unordered_set>

#define HEADERCMP(header, str) \
    case sizeof(str) - 1:      \
        if (!stricmp((header).Name().data(), str))

namespace {
    inline size_t SuggestBufferSize() {
        return 8192;
    }

    inline std::string_view Trim(const char* b, const char* e) noexcept {
        return StripString(std::string_view(b, e));
    }

    inline std::string_view RmSemiColon(const std::string_view& s) {
        return NUtils::Before(s, ';');
    }

    template <class T, size_t N>
    class TStreams: private TNonCopyable {
        struct TDelete {
            inline void operator()(T* t) noexcept {
                delete t;
            }
        };

        typedef T* TPtr;

    public:
        inline TStreams() noexcept
            : Beg_(T_ + N)
        {
        }

        inline ~TStreams() {
            TDelete f;

            ForEach(f);
        }

        template <class S>
        inline S* Add(S* t) noexcept {
            return (S*)AddImpl((T*)t);
        }

        template <class Functor>
        inline void ForEach(Functor& f) {
            const TPtr* end = T_ + N;

            for (TPtr* cur = Beg_; cur != end; ++cur) {
                f(*cur);
            }
        }

        TPtr Top() {
            const TPtr* end = T_ + N;
            return end == Beg_ ? nullptr : *Beg_;
        }

    private:
        inline T* AddImpl(T* t) noexcept {
            Y_ASSERT(Beg_ > T_);

            return (*--Beg_ = t);
        }

    private:
        TPtr T_[N];
        TPtr* Beg_;
    };

    template <class TStream>
    class TLazy: public IOutputStream {
    public:
        TLazy(IOutputStream* out, ui16 bs)
            : Output_(out)
            , BlockSize_(bs)
        {
        }

        void DoWrite(const void* buf, size_t len) override {
            ConstructSlave();
            Slave_->Write(buf, len);
        }

        void DoFlush() override {
            ConstructSlave();
            Slave_->Flush();
        }

        void DoFinish() override {
            ConstructSlave();
            Slave_->Finish();
        }

    private:
        inline void ConstructSlave() {
            if (!Slave_) {
                Slave_.Reset(new TStream(Output_, BlockSize_));
            }
        }

    private:
        IOutputStream* Output_;
        ui16 BlockSize_;
        THolder<IOutputStream> Slave_;
    };
}

class THttpInput::TImpl {
    typedef std::unordered_set<std::string> TAcceptCodings;

public:
    inline TImpl(IInputStream* slave)
        : Slave_(slave)
        , Buffered_(Slave_, SuggestBufferSize())
        , ChunkedInput_(nullptr)
        , Input_(nullptr)
        , FirstLine_(ReadFirstLine(Buffered_))
        , Headers_(&Buffered_)
        , KeepAlive_(false)
        , HasContentLength_(false)
        , ContentLength_(0)
        , ContentEncoded_(false)
        , Expect100Continue_(false)
    {
        BuildInputChain();
        Y_ASSERT(Input_);
    }

    static std::string ReadFirstLine(TBufferedInput& in) {
        std::string s;
        Y_ENSURE_EX(NUtils::ReadLine(in, s), THttpReadException() << "Failed to get first line");
        return s;
    }

    inline ~TImpl() {
    }

    inline size_t Read(void* buf, size_t len) {
        return Perform(len, [this, buf](size_t toRead) { return Input_->Read(buf, toRead); });
    }

    inline size_t Skip(size_t len) {
        return Perform(len, [this](size_t toSkip) { return Input_->Skip(toSkip); });
    }

    inline const std::string& FirstLine() const noexcept {
        return FirstLine_;
    }

    inline const THttpHeaders& Headers() const noexcept {
        return Headers_;
    }

    inline const std::optional<THttpHeaders>& Trailers() const noexcept {
        return Trailers_;
    }

    inline bool IsKeepAlive() const noexcept {
        return KeepAlive_;
    }

    inline bool AcceptEncoding(const std::string& s) const {
        return Codings_.find(NUtils::ToLower(s)) != Codings_.end();
    }

    inline bool GetContentLength(ui64& value) const noexcept {
        if (HasContentLength_) {
            value = ContentLength_;
            return true;
        }
        return false;
    }

    inline bool ContentEncoded() const noexcept {
        return ContentEncoded_;
    }

    inline bool HasContent() const noexcept {
        return HasContentLength_ || ChunkedInput_;
    }

    inline bool HasExpect100Continue() const noexcept {
        return Expect100Continue_;
    }

private:
    template <class Operation>
    inline size_t Perform(size_t len, const Operation& operation) {
        size_t processed = operation(len);
        if (processed == 0 && len > 0) {
            if (!ChunkedInput_) {
                Trailers_.emplace();
            } else {
                // Read the header of the trailing chunk. It remains in
                // the TChunkedInput stream if the HTTP response is compressed.
                char symbol;
                if (ChunkedInput_->Read(&symbol, 1) != 0) {
                    ythrow THttpParseException() << "some data remaining in TChunkedInput";
                }
            }
        }
        return processed;
    }

    struct TParsedHeaders {
        bool Chunked = false;
        bool KeepAlive = false;
        std::string_view LZipped;
    };

    struct TTrEnc {
        inline void operator()(const std::string_view& s) {
            if (s == std::string_view("chunked")) {
                p->Chunked = true;
            }
        }

        TParsedHeaders* p;
    };

    struct TAccCoding {
        inline void operator()(const std::string_view& s) {
            c->insert(ToString(s));
        }

        TAcceptCodings* c;
    };

    template <class Functor>
    inline void ForEach(std::string in, Functor& f) {
        NUtils::ToLower(in);

        const char* b = in.data();
        const char* c = b;
        const char* e = in.data() + in.size();

        while (c != e) {
            if (*c == ',') {
                f(RmSemiColon(Trim(b, c)));
                b = c + 1;
            }

            ++c;
        }

        if (b != c) {
            f(RmSemiColon(Trim(b, c)));
        }
    }

    inline bool IsRequest() const {
        // https://datatracker.ietf.org/doc/html/rfc7231#section-4
        // more rare methods: https://www.iana.org/assignments/http-methods/http-methods.xhtml
        return EqualToOneOf(NUtils::ToLower(FirstLine().substr(0, FirstLine().find(" "))), "get", "post", "put", "head", "delete", "connect", "options", "trace", "patch");
    }

    inline void BuildInputChain() {
        TParsedHeaders p;

        size_t pos = FirstLine_.rfind(' ');
        // In HTTP/1.1 Keep-Alive is turned on by default
        if (pos != std::string::npos && strcmp(FirstLine_.c_str() + pos + 1, "HTTP/1.1") == 0) {
            p.KeepAlive = true; //request
        } else if (strnicmp(FirstLine_.data(), "HTTP/1.1", 8) == 0) {
            p.KeepAlive = true; //reply
        }

        for (THttpHeaders::TConstIterator h = Headers_.Begin(); h != Headers_.End(); ++h) {
            const THttpInputHeader& header = *h;
            switch (header.Name().size()) {
                HEADERCMP(header, "transfer-encoding") {
                    TTrEnc f = {&p};
                    ForEach(header.Value(), f);
                }
                break;
                HEADERCMP(header, "content-encoding") {
                    p.LZipped = header.Value();
                }
                break;
                HEADERCMP(header, "accept-encoding") {
                    TAccCoding f = {&Codings_};
                    ForEach(header.Value(), f);
                }
                break;
                HEADERCMP(header, "content-length") {
                    HasContentLength_ = true;
                    ContentLength_ = std::stoll(header.Value());
                }
                break;
                HEADERCMP(header, "connection") {
                    // accept header "Connection: Keep-Alive, TE"
                    if (strnicmp(header.Value().data(), "keep-alive", 10) == 0) {
                        p.KeepAlive = true;
                    } else if (stricmp(header.Value().data(), "close") == 0) {
                        p.KeepAlive = false;
                    }
                }
                [[fallthrough]];
                HEADERCMP(header, "expect") {
                    auto findContinue = [&](const std::string_view& s) {
                        if (strnicmp(s.data(), "100-continue", 13) == 0) {
                            Expect100Continue_ = true;
                        }
                    };
                    ForEach(header.Value(), findContinue);
                }
                break;
            }
        }

        if (p.Chunked) {
            ChunkedInput_ = Streams_.Add(new TChunkedInput(&Buffered_, &Trailers_));
            Input_ = ChunkedInput_;
        } else {
            // disable buffering
            Buffered_.Reset(&Cnull);
            Input_ = Streams_.Add(new TMultiInput(&Buffered_, Slave_));

            if (IsRequest() || HasContentLength_) {
                /*
                 * TODO - we have other cases
                 */
                Input_ = Streams_.Add(new TLengthLimitedInput(Input_, ContentLength_));
            }
        }

        if (auto decoder = TCompressionCodecFactory::Instance().FindDecoder(p.LZipped)) {
            ContentEncoded_ = true;
            Input_ = Streams_.Add((*decoder)(Input_).Release());
        }

        KeepAlive_ = p.KeepAlive;
    }

private:
    IInputStream* Slave_;

    /*
     * input helpers
     */
    TBufferedInput Buffered_;
    TStreams<IInputStream, 8> Streams_;
    IInputStream* ChunkedInput_;

    /*
     * final input stream
     */
    IInputStream* Input_;

    std::string FirstLine_;
    THttpHeaders Headers_;
    std::optional<THttpHeaders> Trailers_;
    bool KeepAlive_;

    TAcceptCodings Codings_;

    bool HasContentLength_;
    ui64 ContentLength_;

    bool ContentEncoded_;
    bool Expect100Continue_;
};

THttpInput::THttpInput(IInputStream* slave)
    : Impl_(new TImpl(slave))
{
}

THttpInput::THttpInput(THttpInput&& httpInput) = default;

THttpInput::~THttpInput() {
}

size_t THttpInput::DoRead(void* buf, size_t len) {
    return Impl_->Read(buf, len);
}

size_t THttpInput::DoSkip(size_t len) {
    return Impl_->Skip(len);
}

const THttpHeaders& THttpInput::Headers() const noexcept {
    return Impl_->Headers();
}

const std::optional<THttpHeaders>& THttpInput::Trailers() const noexcept {
    return Impl_->Trailers();
}

const std::string& THttpInput::FirstLine() const noexcept {
    return Impl_->FirstLine();
}

bool THttpInput::IsKeepAlive() const noexcept {
    return Impl_->IsKeepAlive();
}

bool THttpInput::AcceptEncoding(const std::string& coding) const {
    return Impl_->AcceptEncoding(coding);
}

std::string THttpInput::BestCompressionScheme(std::span<const std::string_view> codings) const {
    return NHttp::ChooseBestCompressionScheme(
        [this](const std::string& coding) {
            return AcceptEncoding(coding);
        },
        codings
    );
}

std::string THttpInput::BestCompressionScheme() const {
    return BestCompressionScheme(TCompressionCodecFactory::Instance().GetBestCodecs());
}

bool THttpInput::GetContentLength(ui64& value) const noexcept {
    return Impl_->GetContentLength(value);
}

bool THttpInput::ContentEncoded() const noexcept {
    return Impl_->ContentEncoded();
}

bool THttpInput::HasContent() const noexcept {
    return Impl_->HasContent();
}

bool THttpInput::HasExpect100Continue() const noexcept {
    return Impl_->HasExpect100Continue();
}

class THttpOutput::TImpl {
    class TSizeCalculator: public IOutputStream {
    public:
        inline TSizeCalculator() noexcept {
        }

        ~TSizeCalculator() override {
        }

        void DoWrite(const void* /*buf*/, size_t len) override {
            Length_ += len;
        }

        inline size_t Length() const noexcept {
            return Length_;
        }

    private:
        size_t Length_ = 0;
    };

    enum TState {
        Begin = 0,
        FirstLineSent = 1,
        HeadersSent = 2
    };

    struct TFlush {
        inline void operator()(IOutputStream* s) {
            s->Flush();
        }
    };

    struct TFinish {
        inline void operator()(IOutputStream* s) {
            s->Finish();
        }
    };

public:
    inline TImpl(IOutputStream* slave, THttpInput* request)
        : Slave_(slave)
        , State_(Begin)
        , Output_(Slave_)
        , Request_(request)
        , Version_(1100)
        , KeepAliveEnabled_(false)
        , BodyEncodingEnabled_(true)
        , CompressionHeaderEnabled_(true)
        , Finished_(false)
    {
    }

    inline ~TImpl() {
    }

    inline void SendContinue() {
        Output_->Write("HTTP/1.1 100 Continue\r\n\r\n");
        Output_->Flush();
    }

    inline void Write(const void* buf, size_t len) {
        if (Finished_) {
            ythrow THttpException() << "can not write to finished stream";
        }

        if (State_ == HeadersSent) {
            Output_->Write(buf, len);

            return;
        }

        const char* b = (const char*)buf;
        const char* e = b + len;
        const char* c = b;

        while (c != e) {
            if (*c == '\n') {
                Line_.append(b, c);

                if (!Line_.empty() && Line_.back() == '\r') {
                    Line_.pop_back();
                }

                b = c + 1;

                Process(Line_);

                if (State_ == HeadersSent) {
                    Output_->Write(b, e - b);

                    return;
                }

                Line_.clear();
            }

            ++c;
        }

        if (b != c) {
            Line_.append(b, c);
        }
    }

    inline void Flush() {
        TFlush f;
        Streams_.ForEach(f);
        Slave_->Flush(); // see SEARCH-1030
    }

    inline void Finish() {
        if (Finished_) {
            return;
        }

        TFinish f;
        Streams_.ForEach(f);
        Slave_->Finish(); // see SEARCH-1030

        Finished_ = true;
    }

    inline const THttpHeaders& SentHeaders() const noexcept {
        return Headers_;
    }

    inline void EnableCompression(std::span<const std::string_view> schemas) {
        ComprSchemas_ = schemas;
    }

    inline void EnableKeepAlive(bool enable) {
        KeepAliveEnabled_ = enable;
    }

    inline void EnableBodyEncoding(bool enable) {
        BodyEncodingEnabled_ = enable;
    }

    inline void EnableCompressionHeader(bool enable) {
        CompressionHeaderEnabled_ = enable;
    }

    inline bool IsCompressionEnabled() const noexcept {
        return !ComprSchemas_.empty();
    }

    inline bool IsKeepAliveEnabled() const noexcept {
        return KeepAliveEnabled_;
    }

    inline bool IsBodyEncodingEnabled() const noexcept {
        return BodyEncodingEnabled_;
    }

    inline bool IsCompressionHeaderEnabled() const noexcept {
        return CompressionHeaderEnabled_;
    }

    inline bool CanBeKeepAlive() const noexcept {
        return SupportChunkedTransfer() && IsKeepAliveEnabled() && (Request_ ? Request_->IsKeepAlive() : true);
    }

    inline const std::string& FirstLine() const noexcept {
        return FirstLine_;
    }

    inline size_t SentSize() const noexcept {
        return SizeCalculator_.Length();
    }

private:
    static inline bool IsResponse(const std::string& s) noexcept {
        return strnicmp(s.data(), "HTTP/", 5) == 0;
    }

    static inline bool IsRequest(const std::string& s) noexcept {
        return !IsResponse(s);
    }

    inline bool IsHttpRequest() const noexcept {
        return IsRequest(FirstLine_);
    }

    inline bool HasResponseBody() const noexcept {
        if (IsHttpResponse()) {
            if (Request_ && std::string_view(Request_->FirstLine()).starts_with("HEAD"))
                return false;
            if (FirstLine_.size() > 9 && strncmp(FirstLine_.data() + 9, "204", 3) == 0)
                return false;
            return true;
        }
        return false;
    }

    inline bool IsHttpResponse() const noexcept {
        return IsResponse(FirstLine_);
    }

    inline bool HasRequestBody() const noexcept {
        return strnicmp(FirstLine_.data(), "POST", 4) == 0 ||
               strnicmp(FirstLine_.data(), "PATCH", 5) == 0 ||
               strnicmp(FirstLine_.data(), "PUT", 3) == 0;
    }
    static inline size_t ParseHttpVersion(const std::string& s) {
        if (s.empty()) {
            ythrow THttpParseException() << "malformed http stream";
        }

        size_t parsed_version = 0;

        if (IsResponse(s)) {
            const char* b = s.data() + 5;

            while (*b && *b != ' ') {
                if (*b != '.') {
                    parsed_version *= 10;
                    parsed_version += (*b - '0');
                }

                ++b;
            }
        } else {
            /*
             * s not empty here
             */
            const char* e = s.data() + s.size() - 1;
            const char* b = s.data();
            size_t mult = 1;

            while (e != b && *e != '/') {
                if (*e != '.') {
                    parsed_version += (*e - '0') * mult;
                    mult *= 10;
                }

                --e;
            }
        }

        return parsed_version * 100;
    }

    inline void ParseHttpVersion() {
        size_t parsed_version = ParseHttpVersion(FirstLine_);

        if (Request_) {
            parsed_version = Min(parsed_version, ParseHttpVersion(Request_->FirstLine()));
        }

        Version_ = parsed_version;
    }

    inline void Process(const std::string& s) {
        Y_ASSERT(State_ != HeadersSent);

        if (State_ == Begin) {
            FirstLine_ = s;
            ParseHttpVersion();
            State_ = FirstLineSent;
        } else {
            if (s.empty()) {
                BuildOutputStream();
                WriteCached();
                State_ = HeadersSent;
            } else {
                AddHeader(THttpInputHeader(s));
            }
        }
    }

    inline void WriteCachedImpl(IOutputStream* s) const {
        s->Write(FirstLine_.data(), FirstLine_.size());
        s->Write("\r\n", 2);
        Headers_.OutTo(s);
        s->Write("\r\n", 2);
        s->Finish();
    }

    inline void WriteCached() {
        size_t buflen = 0;

        {
            TSizeCalculator out;

            WriteCachedImpl(&out);
            buflen = out.Length();
        }

        {
            TBufferedOutput out(Slave_, buflen);

            WriteCachedImpl(&out);
        }

        if (IsHttpRequest() && !HasRequestBody()) {
            /*
             * if this is http request, then send it now
             */

            Slave_->Flush();
        }
    }

    inline bool SupportChunkedTransfer() const noexcept {
        return Version_ >= 1100;
    }

    inline void BuildOutputStream() {
        if (CanBeKeepAlive()) {
            AddOrReplaceHeader(THttpInputHeader("Connection", "Keep-Alive"));
        } else {
            AddOrReplaceHeader(THttpInputHeader("Connection", "Close"));
        }

        if (IsHttpResponse()) {
            if (Request_ && IsCompressionEnabled() && HasResponseBody()) {
                std::string scheme = Request_->BestCompressionScheme(ComprSchemas_);
                if (scheme != "identity") {
                    AddOrReplaceHeader(THttpInputHeader("Content-Encoding", scheme));
                    RemoveHeader("Content-Length");
                }
            }

            RebuildStream();
        } else {
            if (IsCompressionEnabled()) {
                AddOrReplaceHeader(THttpInputHeader("Accept-Encoding", BuildAcceptEncoding()));
            }
            if (HasRequestBody()) {
                RebuildStream();
            }
        }
    }

    inline std::string BuildAcceptEncoding() const {
        std::string ret;

        for (const auto& coding : ComprSchemas_) {
            if (!ret.empty()) {
                ret += ", ";
            }

            ret += coding;
        }

        return ret;
    }

    inline void RebuildStream() {
        bool keepAlive = false;
        const TCompressionCodecFactory::TEncoderConstructor* encoder = nullptr;
        bool chunked = false;
        bool haveContentLength = false;

        for (THttpHeaders::TConstIterator h = Headers_.Begin(); h != Headers_.End(); ++h) {
            const THttpInputHeader& header = *h;
            const std::string hl = NUtils::ToLower(header.Name());

            if (hl == std::string_view("connection")) {
                keepAlive = NUtils::ToLower(header.Value()) == std::string_view("keep-alive");
            } else if (IsCompressionHeaderEnabled() && hl == std::string_view("content-encoding")) {
                encoder = TCompressionCodecFactory::Instance().FindEncoder(NUtils::ToLower(header.Value()));
            } else if (hl == std::string_view("transfer-encoding")) {
                chunked = NUtils::ToLower(header.Value()) == std::string_view("chunked");
            } else if (hl == std::string_view("content-length")) {
                haveContentLength = true;
            }
        }

        if (!haveContentLength && !chunked && (IsHttpRequest() || HasResponseBody()) && SupportChunkedTransfer() && (keepAlive || encoder || IsHttpRequest())) {
            AddHeader(THttpInputHeader("Transfer-Encoding", "chunked"));
            chunked = true;
        }

        if (IsBodyEncodingEnabled() && chunked) {
            Output_ = Streams_.Add(new TChunkedOutput(Output_));
        }

        Output_ = Streams_.Add(new TTeeOutput(Output_, &SizeCalculator_));

        if (IsBodyEncodingEnabled() && encoder) {
            Output_ = Streams_.Add((*encoder)(Output_).Release());
        }
    }

    inline void AddHeader(const THttpInputHeader& hdr) {
        Headers_.AddHeader(hdr);
    }

    inline void AddOrReplaceHeader(const THttpInputHeader& hdr) {
        Headers_.AddOrReplaceHeader(hdr);
    }

    inline void RemoveHeader(const std::string& hdr) {
        Headers_.RemoveHeader(hdr);
    }

private:
    IOutputStream* Slave_;
    TState State_;
    IOutputStream* Output_;
    TStreams<IOutputStream, 8> Streams_;
    std::string Line_;
    std::string FirstLine_;
    THttpHeaders Headers_;
    THttpInput* Request_;
    size_t Version_;

    std::span<const std::string_view> ComprSchemas_;

    bool KeepAliveEnabled_;
    bool BodyEncodingEnabled_;
    bool CompressionHeaderEnabled_;

    bool Finished_;

    TSizeCalculator SizeCalculator_;
};

THttpOutput::THttpOutput(IOutputStream* slave)
    : Impl_(new TImpl(slave, nullptr))
{
}

THttpOutput::THttpOutput(IOutputStream* slave, THttpInput* request)
    : Impl_(new TImpl(slave, request))
{
}

THttpOutput::~THttpOutput() {
    try {
        Finish();
    } catch (...) {
    }
}

void THttpOutput::DoWrite(const void* buf, size_t len) {
    Impl_->Write(buf, len);
}

void THttpOutput::DoFlush() {
    Impl_->Flush();
}

void THttpOutput::DoFinish() {
    Impl_->Finish();
}

const THttpHeaders& THttpOutput::SentHeaders() const noexcept {
    return Impl_->SentHeaders();
}

void THttpOutput::EnableCompression(bool enable) {
    if (enable) {
        EnableCompression(TCompressionCodecFactory::Instance().GetBestCodecs());
    } else {
        std::span<std::string_view> codings;
        EnableCompression(codings);
    }
}

void THttpOutput::EnableCompression(std::span<const std::string_view> schemas) {
    Impl_->EnableCompression(schemas);
}

void THttpOutput::EnableKeepAlive(bool enable) {
    Impl_->EnableKeepAlive(enable);
}

void THttpOutput::EnableBodyEncoding(bool enable) {
    Impl_->EnableBodyEncoding(enable);
}

void THttpOutput::EnableCompressionHeader(bool enable) {
    Impl_->EnableCompressionHeader(enable);
}

bool THttpOutput::IsKeepAliveEnabled() const noexcept {
    return Impl_->IsKeepAliveEnabled();
}

bool THttpOutput::IsBodyEncodingEnabled() const noexcept {
    return Impl_->IsBodyEncodingEnabled();
}

bool THttpOutput::IsCompressionEnabled() const noexcept {
    return Impl_->IsCompressionEnabled();
}

bool THttpOutput::IsCompressionHeaderEnabled() const noexcept {
    return Impl_->IsCompressionHeaderEnabled();
}

bool THttpOutput::CanBeKeepAlive() const noexcept {
    return Impl_->CanBeKeepAlive();
}

void THttpOutput::SendContinue() {
    Impl_->SendContinue();
}

const std::string& THttpOutput::FirstLine() const noexcept {
    return Impl_->FirstLine();
}

size_t THttpOutput::SentSize() const noexcept {
    return Impl_->SentSize();
}

unsigned ParseHttpRetCode(const std::string_view& ret) {
    const std::string_view code = StripString(NUtils::Before(StripString(NUtils::After(ret, ' ')), ' '));

    return FromString<unsigned>(code.data(), code.size());
}

void SendMinimalHttpRequest(TSocket& s, const std::string_view& host, const std::string_view& request, const std::string_view& agent, const std::string_view& from) {
    TSocketOutput so(s);
    THttpOutput output(&so);

    output.EnableKeepAlive(false);
    output.EnableCompression(false);

    const IOutputStream::TPart parts[] = {
        IOutputStream::TPart(std::string_view("GET ")),
        IOutputStream::TPart(request),
        IOutputStream::TPart(std::string_view(" HTTP/1.1")),
        IOutputStream::TPart::CrLf(),
        IOutputStream::TPart(std::string_view("Host: ")),
        IOutputStream::TPart(host),
        IOutputStream::TPart::CrLf(),
        IOutputStream::TPart(std::string_view("User-Agent: ")),
        IOutputStream::TPart(agent),
        IOutputStream::TPart::CrLf(),
        IOutputStream::TPart(std::string_view("From: ")),
        IOutputStream::TPart(from),
        IOutputStream::TPart::CrLf(),
        IOutputStream::TPart::CrLf(),
    };

    output.Write(parts, sizeof(parts) / sizeof(*parts));
    output.Finish();
}

std::span<const std::string_view> SupportedCodings() {
    return TCompressionCodecFactory::Instance().GetBestCodecs();
}
