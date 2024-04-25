#pragma once

<<<<<<<< HEAD:src/library/monlib/service/pages/mon_page.h
<<<<<<<< HEAD:include/ydb-cpp-sdk/library/monlib/service/pages/mon_page.h
========
>>>>>>>> origin/main:include/ydb-cpp-sdk/library/monlib/service/pages/mon_page.h
#include <ydb-cpp-sdk/library/monlib/service/service.h>
#include <ydb-cpp-sdk/library/monlib/service/mon_service_http_request.h>

#include <string>
#include <ydb-cpp-sdk/util/generic/ptr.h>
<<<<<<<< HEAD:src/library/monlib/service/pages/mon_page.h
========
#include <src/library/monlib/service/service.h>
#include <src/library/monlib/service/mon_service_http_request.h>

#include <string>
#include <src/util/generic/ptr.h>
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/library/monlib/service/pages/mon_page.h
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/library/monlib/service/pages/mon_page.h
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
========
>>>>>>>> origin/main:include/ydb-cpp-sdk/library/monlib/service/pages/mon_page.h

namespace NMonitoring {
    static const char HTTPOKTEXT[] = "HTTP/1.1 200 Ok\r\nContent-Type: text/plain\r\nConnection: Close\r\n\r\n";
    static const char HTTPOKBIN[] = "HTTP/1.1 200 Ok\r\nContent-Type: application/octet-stream\r\nConnection: Close\r\n\r\n";
    static const char HTTPOKHTML[] = "HTTP/1.1 200 Ok\r\nContent-Type: text/html\r\nConnection: Close\r\n\r\n";
    static const char HTTPOKJSON[] = "HTTP/1.1 200 Ok\r\nContent-Type: application/json\r\nConnection: Close\r\n\r\n";
    static const char HTTPOKSPACK[] = "HTTP/1.1 200 Ok\r\nContent-Type: application/x-solomon-spack\r\nConnection: Close\r\n\r\n";
    static const char HTTPOKPROMETHEUS[] = "HTTP/1.1 200 Ok\r\nContent-Type: text/plain\r\nConnection: Close\r\n\r\n";
    static const char HTTPOKUNISTAT[] = "HTTP/1.1 200 Ok\r\nContent-Type: text/json\r\nConnection: Close\r\n\r\n";
    static const char HTTPOKJAVASCRIPT[] = "HTTP/1.1 200 Ok\r\nContent-Type: text/javascript\r\nConnection: Close\r\n\r\n";
    static const char HTTPOKCSS[] = "HTTP/1.1 200 Ok\r\nContent-Type: text/css\r\nConnection: Close\r\n\r\n";
    static const char HTTPNOCONTENT[] = "HTTP/1.1 204 No content\r\nConnection: Close\r\n\r\n";
    static const char HTTPNOTFOUND[] = "HTTP/1.1 404 Invalid URI\r\nConnection: Close\r\n\r\nInvalid URL\r\n";
    static const char HTTPUNAUTHORIZED[] = "HTTP/1.1 401 Unauthorized\r\nConnection: Close\r\n\r\nUnauthorized\r\n";
    static const char HTTPFORBIDDEN[] = "HTTP/1.1 403 Forbidden\r\nConnection: Close\r\n\r\nForbidden\r\n";
    static const char HTTPOKJAVASCRIPT_CACHED[] = "HTTP/1.1 200 Ok\r\nContent-Type: text/javascript\r\nCache-Control: public, max-age=31536000\r\nConnection: Close\r\n\r\n";
    static const char HTTPOKCSS_CACHED[] = "HTTP/1.1 200 Ok\r\nContent-Type: text/css\r\nCache-Control: public, max-age=31536000\r\nConnection: Close\r\n\r\n";

    // Fonts
    static const char HTTPOKFONTEOT[] = "HTTP/1.1 200 Ok\r\nContent-Type: application/vnd.ms-fontobject\r\nConnection: Close\r\n\r\n";
    static const char HTTPOKFONTTTF[] = "HTTP/1.1 200 Ok\r\nContent-Type: application/x-font-ttf\r\nConnection: Close\r\n\r\n";
    static const char HTTPOKFONTWOFF[] = "HTTP/1.1 200 Ok\r\nContent-Type: application/font-woff\r\nConnection: Close\r\n\r\n";
    static const char HTTPOKFONTWOFF2[] = "HTTP/1.1 200 Ok\r\nContent-Type: application/font-woff2\r\nConnection: Close\r\n\r\n";
    static const char HTTPOKFONTEOT_CACHED[] = "HTTP/1.1 200 Ok\r\nContent-Type: application/vnd.ms-fontobject\r\nCache-Control: public, max-age=31536000\r\nConnection: Close\r\n\r\n";
    static const char HTTPOKFONTTTF_CACHED[] = "HTTP/1.1 200 Ok\r\nContent-Type: application/x-font-ttf\r\nCache-Control: public, max-age=31536000\r\nConnection: Close\r\n\r\n";
    static const char HTTPOKFONTWOFF_CACHED[] = "HTTP/1.1 200 Ok\r\nContent-Type: application/font-woff\r\nCache-Control: public, max-age=31536000\r\nConnection: Close\r\n\r\n";
    static const char HTTPOKFONTWOFF2_CACHED[] = "HTTP/1.1 200 Ok\r\nContent-Type: application/font-woff2\r\nCache-Control: public, max-age=31536000\r\nConnection: Close\r\n\r\n";

    // Images
    static const char HTTPOKPNG[] = "HTTP/1.1 200 Ok\r\nContent-Type: image/png\r\nConnection: Close\r\n\r\n";
    static const char HTTPOKSVG[] = "HTTP/1.1 200 Ok\r\nContent-Type: image/svg+xml\r\nConnection: Close\r\n\r\n";

    class IMonPage;

    using TMonPagePtr = TIntrusivePtr<IMonPage>;

    class IMonPage: public TAtomicRefCount<IMonPage> {
    public:
        const std::string Path;
        const std::string Title;
        const IMonPage* Parent = nullptr;

    public:
        IMonPage(const std::string& path, const std::string& title = std::string());

        virtual ~IMonPage() {
        }

        void OutputNavBar(IOutputStream& out);

        virtual const std::string& GetPath() const {
            return Path;
        }

        virtual const std::string& GetTitle() const {
            return Title;
        }

        bool IsInIndex() const {
            return !Title.empty();
        }

        virtual bool IsIndex() const {
            return false;
        }

        virtual void Output(IMonHttpRequest& request) = 0;
    };

}
