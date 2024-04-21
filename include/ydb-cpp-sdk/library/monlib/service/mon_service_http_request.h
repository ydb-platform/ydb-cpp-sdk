#pragma once

#include "service.h"

#include <src/util/stream/output.h>

namespace NMonitoring {
    class TMonService2;
    class IMonPage;

    // XXX: IHttpRequest is already taken
    struct IMonHttpRequest {
        virtual ~IMonHttpRequest();

        virtual IOutputStream& Output() = 0;

        virtual HTTP_METHOD GetMethod() const = 0;
        virtual std::string_view GetPath() const = 0;
        virtual std::string_view GetPathInfo() const = 0;
        virtual std::string_view GetUri() const = 0;
        virtual const TCgiParameters& GetParams() const = 0;
        virtual const TCgiParameters& GetPostParams() const = 0;
        virtual std::string_view GetPostContent() const = 0;
        virtual const THttpHeaders& GetHeaders() const = 0;
        virtual std::string_view GetHeader(std::string_view name) const = 0;
        virtual std::string_view GetCookie(std::string_view name) const = 0;
        virtual std::string GetRemoteAddr() const = 0;

        virtual std::string GetServiceTitle() const = 0;

        virtual IMonPage* GetPage() const = 0;

        virtual IMonHttpRequest* MakeChild(IMonPage* page, const std::string& pathInfo) const = 0;
    };

    struct TMonService2HttpRequest: IMonHttpRequest {
        IOutputStream* const Out;
        const IHttpRequest* const HttpRequest;
        TMonService2* const MonService;
        IMonPage* const MonPage;
        const std::string PathInfo;
        TMonService2HttpRequest* const Parent;

        TMonService2HttpRequest(
            IOutputStream* out, const IHttpRequest* httpRequest,
            TMonService2* monService, IMonPage* monPage,
            const std::string& pathInfo,
            TMonService2HttpRequest* parent)
            : Out(out)
            , HttpRequest(httpRequest)
            , MonService(monService)
            , MonPage(monPage)
            , PathInfo(pathInfo)
            , Parent(parent)
        {
        }

        ~TMonService2HttpRequest() override;

        IOutputStream& Output() override;
        HTTP_METHOD GetMethod() const override;
        std::string_view GetPath() const override;
        std::string_view GetPathInfo() const override;
        std::string_view GetUri() const override;
        const TCgiParameters& GetParams() const override;
        const TCgiParameters& GetPostParams() const override;
        std::string_view GetPostContent() const override {
            return HttpRequest->GetPostContent();
        }

        std::string_view GetHeader(std::string_view name) const override;
        std::string_view GetCookie(std::string_view name) const override;
        const THttpHeaders& GetHeaders() const override;
        std::string GetRemoteAddr() const override;

        IMonPage* GetPage() const override {
            return MonPage;
        }

        TMonService2HttpRequest* MakeChild(IMonPage* page, const std::string& pathInfo) const override {
            return new TMonService2HttpRequest{
                Out, HttpRequest, MonService, page,
                pathInfo, const_cast<TMonService2HttpRequest*>(this)
            };
        }

        std::string GetServiceTitle() const override;
    };

}
