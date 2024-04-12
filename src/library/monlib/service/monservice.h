#pragma once

#include "service.h"
#include "auth.h"
#include "mon_service_http_request.h"

#include <src/library/monlib/service/pages/index_mon_page.h>
#include <src/library/monlib/service/pages/mon_page.h>

#include <src/util/system/progname.h>

#include <functional>

namespace NMonitoring {
    class TMonService2: public TMtHttpServer {
    protected:
        const std::string Title;
        char StartTime[26];
        TIntrusivePtr<TIndexMonPage> IndexMonPage;
        THolder<IAuthProvider> AuthProvider_;

    public:
        static THttpServerOptions HttpServerOptions(ui16 port, const std::string& host, ui32 threads) {
            THttpServerOptions opts(port);
            if (!host.empty()) {
                opts.SetHost(host);
            }
            opts.SetClientTimeout(TDuration::Minutes(1));
            opts.EnableCompression(true);
            opts.SetThreads(threads);
            opts.SetMaxConnections(std::max<ui32>(100, threads));
            opts.EnableRejectExcessConnections(true);
            return opts;
        }

        static THttpServerOptions HttpServerOptions(ui16 port, ui32 threads) {
            return HttpServerOptions(port, std::string(), threads);
        }

    public:
        explicit TMonService2(ui16 port, const std::string& title = GetProgramName(), THolder<IAuthProvider> auth = nullptr);
        explicit TMonService2(ui16 port, ui32 threads, const std::string& title = GetProgramName(), THolder<IAuthProvider> auth = nullptr);
        explicit TMonService2(ui16 port, const std::string& host, ui32 threads, const std::string& title = GetProgramName(), THolder<IAuthProvider> auth = nullptr);
        explicit TMonService2(const THttpServerOptions& options, const std::string& title = GetProgramName(), THolder<IAuthProvider> auth = nullptr);
        explicit TMonService2(const THttpServerOptions& options, TSimpleSharedPtr<IThreadPool> pool, const std::string& title = GetProgramName(), THolder<IAuthProvider> auth = nullptr);

        ~TMonService2() override {
        }

        const char* GetStartTime() const {
            return StartTime;
        }

        const std::string& GetTitle() const {
            return Title;
        }

        virtual void ServeRequest(IOutputStream& out, const NMonitoring::IHttpRequest& request);
        virtual void OutputIndex(IOutputStream& out);
        virtual void OutputIndexPage(IOutputStream& out);
        virtual void OutputIndexBody(IOutputStream& out);

        void Register(IMonPage* page);
        void Register(TMonPagePtr page);

        TIndexMonPage* RegisterIndexPage(const std::string& path, const std::string& title);

        IMonPage* FindPage(const std::string& relativePath);
        TIndexMonPage* FindIndexPage(const std::string& relativePath);
        void SortPages();

        TIndexMonPage* GetRoot() {
            return IndexMonPage.Get();
        }
    };

}
