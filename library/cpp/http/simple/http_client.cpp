#include "http_client.h"

#include <library/cpp/string_utils/url/url.h>
#include <library/cpp/uri/http_url.h>

#include <util/string/join.h>
#include <util/string/split.h>

TKeepAliveHttpClient::TKeepAliveHttpClient(const std::string& host,
                                           ui32 port,
                                           TDuration socketTimeout,
                                           TDuration connectTimeout)
    : Host(CutHttpPrefix(host))
    , Port(port)
    , SocketTimeout(socketTimeout)
    , ConnectTimeout(connectTimeout)
    , IsHttps(std::string_view{host}.starts_with("https"))
    , IsClosingRequired(false)
    , HttpsVerification(TVerifyCert{Host})
    , IfResponseRequired([](const THttpInput&) { return true; })
{
}

TKeepAliveHttpClient::THttpCode TKeepAliveHttpClient::DoGet(const std::string_view relativeUrl,
                                                            IOutputStream* output,
                                                            const THeaders& headers,
                                                            THttpHeaders* outHeaders) {
    return DoRequest(std::string_view("GET"),
                     relativeUrl,
                     {},
                     output,
                     headers,
                     outHeaders);
}

TKeepAliveHttpClient::THttpCode TKeepAliveHttpClient::DoPost(const std::string_view relativeUrl,
                                                             const std::string_view body,
                                                             IOutputStream* output,
                                                             const THeaders& headers,
                                                             THttpHeaders* outHeaders) {
    return DoRequest(std::string_view("POST"),
                     relativeUrl,
                     body,
                     output,
                     headers,
                     outHeaders);
}

TKeepAliveHttpClient::THttpCode TKeepAliveHttpClient::DoRequest(const std::string_view method,
                                                                const std::string_view relativeUrl,
                                                                const std::string_view body,
                                                                IOutputStream* output,
                                                                const THeaders& inHeaders,
                                                                THttpHeaders* outHeaders) {
    const std::string contentLength = IntToString<10, size_t>(body.size());
    return DoRequestReliable(FormRequest(method, relativeUrl, body, inHeaders, contentLength), output, outHeaders);
}

TKeepAliveHttpClient::THttpCode TKeepAliveHttpClient::DoRequestRaw(const std::string_view raw,
                                                                   IOutputStream* output,
                                                                   THttpHeaders* outHeaders) {
    return DoRequestReliable(raw, output, outHeaders);
}

void TKeepAliveHttpClient::DisableVerificationForHttps() {
    HttpsVerification.reset();
    Connection.Reset();
}

void TKeepAliveHttpClient::SetClientCertificate(const TOpenSslClientIO::TOptions::TClientCert& options) {
    ClientCertificate = options;
}

void TKeepAliveHttpClient::ResetConnection() {
    Connection.Reset();
}

std::vector<IOutputStream::TPart> TKeepAliveHttpClient::FormRequest(std::string_view method,
                                                                const std::string_view relativeUrl,
                                                                std::string_view body,
                                                                const TKeepAliveHttpClient::THeaders& headers,
                                                                std::string_view contentLength) const {
    std::vector<IOutputStream::TPart> parts;

    parts.reserve(16 + 4 * headers.size());
    parts.push_back(TStringBuf(method));
    parts.push_back(TStringBuf(" "));
    parts.push_back(TStringBuf(relativeUrl));
    parts.push_back(TStringBuf(" HTTP/1.1"));
    parts.push_back(IOutputStream::TPart::CrLf());
    parts.push_back(TStringBuf("Host: "));
    parts.push_back(TStringBuf(Host));
    parts.push_back(IOutputStream::TPart::CrLf());
    parts.push_back(TStringBuf("Content-Length: "));
    parts.push_back(TStringBuf(contentLength));
    parts.push_back(IOutputStream::TPart::CrLf());

    for (const auto& entry : headers) {
        parts.push_back(IOutputStream::TPart(entry.first));
        parts.push_back(IOutputStream::TPart(std::string_view(": ")));
        parts.push_back(IOutputStream::TPart(entry.second));
        parts.push_back(IOutputStream::TPart::CrLf());
    }

    parts.push_back(IOutputStream::TPart::CrLf());
    if (!body.empty()) {
        parts.push_back(IOutputStream::TPart(body));
    }

    return parts;
}

TKeepAliveHttpClient::THttpCode TKeepAliveHttpClient::ReadAndTransferHttp(THttpInput& input,
                                                                          IOutputStream* output,
                                                                          THttpHeaders* outHeaders) const {
    TKeepAliveHttpClient::THttpCode statusCode;
    try {
        statusCode = ParseHttpRetCode(input.FirstLine());
    } catch (TFromStringException& e) {
        std::string rest = input.ReadAll();
        ythrow THttpRequestException() << "Failed parse status code in response of " << Host << ": " << e.what() << " (" << input.FirstLine() << ")"
                                       << "\nFull http response:\n"
                                       << rest;
    }

    auto canContainBody = [](auto statusCode) {
        return statusCode != HTTP_NOT_MODIFIED && statusCode != HTTP_NO_CONTENT;
    };

    if (output && canContainBody(statusCode) && IfResponseRequired(input)) {
        TransferData(&input, output);
    }
    if (outHeaders) {
        *outHeaders = input.Headers();
    }

    return statusCode;
}

THttpInput* TKeepAliveHttpClient::GetHttpInput() {
    return Connection ? Connection->GetHttpInput() : nullptr;
}

bool TKeepAliveHttpClient::CreateNewConnectionIfNeeded() {
    if (IsClosingRequired || (Connection && !Connection->IsOk())) {
        Connection.Reset();
    }
    if (!Connection) {
        Connection = MakeHolder<NPrivate::THttpConnection>(Host,
                                                           Port,
                                                           SocketTimeout,
                                                           ConnectTimeout,
                                                           IsHttps,
                                                           ClientCertificate,
                                                           HttpsVerification);
        IsClosingRequired = false;
        return true;
    }
    return false;
}

THttpRequestException::THttpRequestException(int statusCode)
    : StatusCode(statusCode)
{
}

int THttpRequestException::GetStatusCode() const {
    return StatusCode;
}

TSimpleHttpClient::TSimpleHttpClient(const TOptions& options)
    : Host(options.Host())
    , Port(options.Port())
    , SocketTimeout(options.SocketTimeout())
    , ConnectTimeout(options.ConnectTimeout())
{
}

TSimpleHttpClient::TSimpleHttpClient(const std::string& host, ui32 port, TDuration socketTimeout, TDuration connectTimeout)
    : Host(host)
    , Port(port)
    , SocketTimeout(socketTimeout)
    , ConnectTimeout(connectTimeout)
{
}

void TSimpleHttpClient::EnableVerificationForHttps() {
    HttpsVerification = true;
}

void TSimpleHttpClient::DoGet(const std::string_view relativeUrl, IOutputStream* output, const THeaders& headers) const {
    TKeepAliveHttpClient cl = CreateClient();

    TKeepAliveHttpClient::THttpCode code = cl.DoGet(relativeUrl, output, headers);

    Y_ENSURE(cl.GetHttpInput());
    ProcessResponse(relativeUrl, *cl.GetHttpInput(), output, code);
}

void TSimpleHttpClient::DoPost(const std::string_view relativeUrl, std::string_view body, IOutputStream* output, const THashMap<std::string, std::string>& headers) const {
    TKeepAliveHttpClient cl = CreateClient();

    TKeepAliveHttpClient::THttpCode code = cl.DoPost(relativeUrl, body, output, headers);

    Y_ENSURE(cl.GetHttpInput());
    ProcessResponse(relativeUrl, *cl.GetHttpInput(), output, code);
}

void TSimpleHttpClient::DoPostRaw(const std::string_view relativeUrl, const std::string_view rawRequest, IOutputStream* output) const {
    TKeepAliveHttpClient cl = CreateClient();

    TKeepAliveHttpClient::THttpCode code = cl.DoRequestRaw(rawRequest, output);

    Y_ENSURE(cl.GetHttpInput());
    ProcessResponse(relativeUrl, *cl.GetHttpInput(), output, code);
}

namespace NPrivate {
    THttpConnection::THttpConnection(const std::string& host,
                                     ui32 port,
                                     TDuration sockTimeout,
                                     TDuration connTimeout,
                                     bool isHttps,
                                     const std::optional<TOpenSslClientIO::TOptions::TClientCert>& clientCert,
                                     const std::optional<TOpenSslClientIO::TOptions::TVerifyCert>& verifyCert)
        : Addr(Resolve(host, port))
        , Socket(Connect(Addr, sockTimeout, connTimeout, host, port))
        , SocketIn(Socket)
        , SocketOut(Socket)
    {
        if (isHttps) {
            TOpenSslClientIO::TOptions opts;
            if (clientCert) {
                opts.ClientCert_ = clientCert;
            }
            if (verifyCert) {
                opts.VerifyCert_ = verifyCert;
            }

            Ssl = MakeHolder<TOpenSslClientIO>(&SocketIn, &SocketOut, opts);
            HttpOut = MakeHolder<THttpOutput>(Ssl.Get());
        } else {
            HttpOut = MakeHolder<THttpOutput>(&SocketOut);
        }

        HttpOut->EnableKeepAlive(true);
    }

    TNetworkAddress THttpConnection::Resolve(const std::string& host, ui32 port) {
        try {
            return TNetworkAddress(host.c_str(), port);
        } catch (const yexception& e) {
            ythrow THttpRequestException() << "Resolve of " << host << ": " << e.what();
        }
    }

    TSocket THttpConnection::Connect(TNetworkAddress& addr,
                                     TDuration sockTimeout,
                                     TDuration connTimeout,
                                     const std::string& host,
                                     ui32 port) {
        try {
            TSocket socket(addr, connTimeout);
            TDuration socketTimeout = Max(sockTimeout, TDuration::MilliSeconds(1)); // timeout less than 1ms will be interpreted as 0 in SetSocketTimeout() call below and will result in infinite wait

            ui32 seconds = socketTimeout.Seconds();
            ui32 milliSeconds = (socketTimeout - TDuration::Seconds(seconds)).MilliSeconds();
            socket.SetSocketTimeout(seconds, milliSeconds);
            return socket;
        } catch (const yexception& e) {
            ythrow THttpRequestException() << "Connect to " << host << ':' << port << " failed: " << e.what();
        }
    }
}

void TSimpleHttpClient::ProcessResponse(const std::string_view relativeUrl, THttpInput& input, IOutputStream*, const unsigned statusCode) const {
    if (!(statusCode >= 200 && statusCode < 300)) {
        std::string rest = input.ReadAll();
        ythrow THttpRequestException(statusCode) << "Got " << statusCode << " at " << Host << relativeUrl << "\nFull http response:\n"
                                                 << rest;
    }
}

TSimpleHttpClient::~TSimpleHttpClient() {
}

TKeepAliveHttpClient TSimpleHttpClient::CreateClient() const {
    TKeepAliveHttpClient cl(Host, Port, SocketTimeout, ConnectTimeout);

    if (!HttpsVerification) {
        cl.DisableVerificationForHttps();
    }

    PrepareClient(cl);

    return cl;
}

void TSimpleHttpClient::PrepareClient(TKeepAliveHttpClient&) const {
}

TRedirectableHttpClient::TRedirectableHttpClient(const std::string& host, ui32 port, TDuration socketTimeout, TDuration connectTimeout)
    : TSimpleHttpClient(host, port, socketTimeout, connectTimeout)
{
}

void TRedirectableHttpClient::PrepareClient(TKeepAliveHttpClient& cl) const {
    cl.IfResponseRequired = [](const THttpInput& input) {
        return !input.Headers().HasHeader("Location");
    };
}

void TRedirectableHttpClient::ProcessResponse(const std::string_view relativeUrl, THttpInput& input, IOutputStream* output, const unsigned statusCode) const {
    for (auto i = input.Headers().Begin(), e = input.Headers().End(); i != e; ++i) {
        if (i->Name() == "Location") {
            std::vector<std::string> request_url_parts, request_body_parts;

            size_t splitted_index = 0;
            for (auto& iter : StringSplitter(i->Value()).Split('/')) {
                if (splitted_index < 3) {
                    request_url_parts.push_back(std::string(iter.Token()));
                } else {
                    request_body_parts.push_back(std::string(iter.Token()));
                }
                ++splitted_index;
            }

            std::string url = JoinSeq("/", request_url_parts);
            ui16 port = 443;

            THttpURL u;
            if (THttpURL::ParsedOK == u.Parse(url)) {
                const char* p = u.Get(THttpURL::FieldPort);
                if (p) {
                    port = FromString<ui16>(p);
                    url = u.PrintS(THttpURL::FlagScheme | THttpURL::FlagHost);
                }
            }

            TRedirectableHttpClient cl(url, port, TDuration::Seconds(60), TDuration::Seconds(60));
            if (HttpsVerification) {
                cl.EnableVerificationForHttps();
            }
            cl.DoGet(std::string("/") + JoinSeq("/", request_body_parts), output);
            return;
        }
    }
    if (!(statusCode >= 200 && statusCode < 300)) {
        std::string rest = input.ReadAll();
        ythrow THttpRequestException(statusCode) << "Got " << statusCode << " at " << Host << relativeUrl << "\nFull http response:\n"
                                                 << rest;
    }
    TransferData(&input, output);
}
