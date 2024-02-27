#include "mon_service_http_request.h"
#include "monservice.h"

using namespace NMonitoring;

IMonHttpRequest::~IMonHttpRequest() {
}

TMonService2HttpRequest::~TMonService2HttpRequest() {
}

std::string TMonService2HttpRequest::GetServiceTitle() const {
    return MonService->GetTitle();
}

IOutputStream& TMonService2HttpRequest::Output() {
    return *Out;
}

HTTP_METHOD TMonService2HttpRequest::GetMethod() const {
    return HttpRequest->GetMethod();
}

std::string_view TMonService2HttpRequest::GetPathInfo() const {
    return PathInfo;
}

std::string_view TMonService2HttpRequest::GetPath() const {
    return HttpRequest->GetPath();
}

std::string_view TMonService2HttpRequest::GetUri() const {
    return HttpRequest->GetURI();
}

const TCgiParameters& TMonService2HttpRequest::GetParams() const {
    return HttpRequest->GetParams();
}

const TCgiParameters& TMonService2HttpRequest::GetPostParams() const {
    return HttpRequest->GetPostParams();
}

std::string_view TMonService2HttpRequest::GetHeader(std::string_view name) const {
    const THttpHeaders& headers = HttpRequest->GetHeaders();
    const THttpInputHeader* header = headers.FindHeader(name);
    if (header != nullptr) {
        return header->Value();
    }
    return std::string_view();
}

const THttpHeaders& TMonService2HttpRequest::GetHeaders() const {
    return HttpRequest->GetHeaders();
}

std::string TMonService2HttpRequest::GetRemoteAddr() const {
    return HttpRequest->GetRemoteAddr();
}

std::string_view TMonService2HttpRequest::GetCookie(std::string_view name) const {
    std::string_view cookie = GetHeader("Cookie");
    size_t size = cookie.size();
    size_t start = 0;
    while (start < size) {
        size_t semicolon = cookie.find(';', start);
        auto pair = cookie.substr(start, semicolon - start);
        if (!pair.empty()) {
            size_t equal = pair.find('=');
            if (equal != std::string_view::npos) {
                auto cookieName = pair.substr(0, equal);
                if (cookieName == name) {
                    size_t valueStart = equal + 1;
                    auto cookieValue = pair.substr(valueStart, semicolon - valueStart);
                    return cookieValue;
                }
            }
            start = semicolon;
            while (start < size && (cookie[start] == ' ' || cookie[start] == ';')) {
                ++start;
            }
        }
    }
    return std::string_view();
}
