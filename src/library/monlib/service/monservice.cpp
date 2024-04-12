#include "monservice.h"

#include <src/library/string_utils/base64/base64.h>
#include <src/library/svnversion/svnversion.h>

#include <src/util/generic/ptr.h>
#include <src/util/system/hostname.h>

#include <google/protobuf/text_format.h>

using namespace NMonitoring;

TMonService2::TMonService2(ui16 port, const std::string& host, ui32 threads, const std::string& title, THolder<IAuthProvider> auth)
    : TMonService2(HttpServerOptions(port, host, threads), title, std::move(auth))
{
}

TMonService2::TMonService2(const THttpServerOptions& options, const std::string& title, THolder<IAuthProvider> auth)
    : NMonitoring::TMtHttpServer(options, std::bind(&TMonService2::ServeRequest, this, std::placeholders::_1, std::placeholders::_2))
    , Title(title)
    , IndexMonPage(new TIndexMonPage("", Title))
    , AuthProvider_{std::move(auth)}
{
    Y_ABORT_UNLESS(!title.empty());
    time_t t = time(nullptr);
    ctime_r(&t, StartTime);
}

TMonService2::TMonService2(const THttpServerOptions& options, TSimpleSharedPtr<IThreadPool> pool, const std::string& title, THolder<IAuthProvider> auth)
    : NMonitoring::TMtHttpServer(options, std::bind(&TMonService2::ServeRequest, this, std::placeholders::_1, std::placeholders::_2), std::move(pool))
    , Title(title)
    , IndexMonPage(new TIndexMonPage("", Title))
    , AuthProvider_{std::move(auth)}
{
    Y_ABORT_UNLESS(!title.empty());
    time_t t = time(nullptr);
    ctime_r(&t, StartTime);
}

TMonService2::TMonService2(ui16 port, ui32 threads, const std::string& title, THolder<IAuthProvider> auth)
    : TMonService2(port, std::string(), threads, title, std::move(auth))
{
}

TMonService2::TMonService2(ui16 port, const std::string& title, THolder<IAuthProvider> auth)
    : TMonService2(port, std::string(), 0, title, std::move(auth))
{
}

void TMonService2::OutputIndex(IOutputStream& out) {
    IndexMonPage->OutputIndex(out, true);
}

void TMonService2::OutputIndexPage(IOutputStream& out) {
    out << HTTPOKHTML;
    out << "<html>\n";
    IndexMonPage->OutputHead(out);
    OutputIndexBody(out);
    out << "</html>\n";
}

void TMonService2::OutputIndexBody(IOutputStream& out) {
    out << "<body>\n";

    // part of common navbar
    out << "<ol class='breadcrumb'>\n";
    out << "<li class='active'>" << Title << "</li>\n";
    out << "</ol>\n";

    out << "<div class='container'>\n"
        << "<h2>" << Title << "</h2>\n";
    OutputIndex(out);
    out
        << "<div>\n"
        << "</body>\n";
}

void TMonService2::ServeRequest(IOutputStream& out, const NMonitoring::IHttpRequest& request) {
    std::string path = request.GetPath();
    Y_ABORT_UNLESS(path.starts_with('/'));

    if (AuthProvider_) {
        const auto authResult = AuthProvider_->Check(request);
        switch (authResult.Status) {
            case TAuthResult::EStatus::NoCredentials:
                out << HTTPUNAUTHORIZED;
                return;
            case TAuthResult::EStatus::Denied:
                out << HTTPFORBIDDEN;
                return;
            case TAuthResult::EStatus::Ok:
                break;
        }
    }

    if (path == "/") {
        OutputIndexPage(out);
    } else {
        TMonService2HttpRequest monService2HttpRequest(
            &out, &request, this, IndexMonPage.Get(), path, nullptr);
        IndexMonPage->Output(monService2HttpRequest);
    }
}

void TMonService2::Register(IMonPage* page) {
    IndexMonPage->Register(page);
}

void TMonService2::Register(TMonPagePtr page) {
    IndexMonPage->Register(std::move(page));
}

TIndexMonPage* TMonService2::RegisterIndexPage(const std::string& path, const std::string& title) {
    return IndexMonPage->RegisterIndexPage(path, title);
}

IMonPage* TMonService2::FindPage(const std::string& relativePath) {
    return IndexMonPage->FindPage(relativePath);
}

TIndexMonPage* TMonService2::FindIndexPage(const std::string& relativePath) {
    return IndexMonPage->FindIndexPage(relativePath);
}

void TMonService2::SortPages() {
    IndexMonPage->SortPages();
}
