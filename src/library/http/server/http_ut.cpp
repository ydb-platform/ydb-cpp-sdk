#include <ydb-cpp-sdk/library/http/server/http.h>
#include "http_ex.h"

#include <src/library/testing/unittest/registar.h>
#include <src/library/testing/unittest/tests_data.h>

#include <ydb-cpp-sdk/util/generic/cast.h>
#include <ydb-cpp-sdk/util/stream/output.h>
#include <src/util/stream/zlib.h>
#include <src/util/system/datetime.h>
#include <ydb-cpp-sdk/util/random/random.h>

#include <mutex>

Y_UNIT_TEST_SUITE(THttpServerTest) {
    class TEchoServer: public THttpServer::ICallBack {
        class TRequest: public THttpClientRequestEx {
        public:
            inline TRequest(TEchoServer* parent)
                : Parent_(parent)
            {
            }

            bool Reply(void* /*tsr*/) override {
                if (!ProcessHeaders()) {
                    return true;
                }

                Output() << "HTTP/1.1 200 Ok\r\n\r\n";
                if (Buf.Size()) {
                    Output().Write(Buf.AsCharPtr(), Buf.Size());
                } else {
                    Output() << Parent_->Res_;
                }
                Output().Finish();

                return true;
            }

        private:
            TEchoServer* Parent_ = nullptr;
        };

    public:
        inline TEchoServer(const std::string& res)
            : Res_(res)
        {
        }

        TClientRequest* CreateClient() override {
            return new TRequest(this);
        }

    private:
        std::string Res_;
    };

    class TSleepingServer: public THttpServer::ICallBack {
        class TReplier: public TRequestReplier {
        public:
            inline TReplier(TSleepingServer* server)
                : Server(server)
            {
            }

            bool BeforeParseRequestOk(void*) override {
                if (Server->Ttl && (TInstant::Now() - CreateTime > TDuration::MilliSeconds(Server->Ttl))) {
                    Output().Write("HTTP/1.0 503 Created\nX-Server: sleeping server\n\nTTL Exceed");
                    return false;
                } else {
                    return true;
                }
            }

            bool DoReply(const TReplyParams& params) override {
                ++Server->Replies;
                std::lock_guard guard(Server->Lock);
                params.Output.Write("HTTP/1.0 201 Created\nX-Server: sleeping server\n\nZoooo");
                params.Output.Finish();
                return true;
            }

            using TClientRequest::Output;

        private:
            TSleepingServer* Server = nullptr;
            TInstant CreateTime = TInstant::Now();
        };

    public:
        TSleepingServer(size_t ttl = 0)
        : Ttl(ttl) {}

        TClientRequest* CreateClient() override {
            return new TReplier(this);
        }

        void OnMaxConn() override {
            ++MaxConns;
        }
    public:
        std::mutex Lock;

        std::atomic<size_t> Replies;
        std::atomic<size_t> MaxConns;
        size_t Ttl;
    };

    static const std::string CrLf = "\r\n";

    struct TTestRequest {
        TTestRequest(ui16 port, std::string content = std::string())
            : Port(port)
            , Content(std::move(content))
        {
        }

        void CheckContinue(TSocketInput& si) {
            if (Expect100Continue) {
                std::stringStream ss;
                std::string firstLine;
                si.ReadLine(firstLine);
                for (;;) {
                    std::string buf;
                    si.ReadLine(buf);
                    if (buf.size() == 0) {
                        break;
                    }
                    ss << buf << CrLf;
                }
                UNIT_ASSERT_EQUAL(firstLine, "HTTP/1.1 100 Continue");
            }
        }

        std::string Execute() {
            TSocket* s = nullptr;
            THolder<TSocket> singleReqSocket;
            if (KeepAliveConnection) {
                if (!KeepAlivedSocket) {
                    KeepAlivedSocket = MakeHolder<TSocket>(TNetworkAddress("localhost", Port), TDuration::Seconds(10));
                }
                s = KeepAlivedSocket.Get();
            } else {
                TNetworkAddress addr("localhost", Port);
                singleReqSocket.Reset(new TSocket(addr, TDuration::Seconds(10)));
                s = singleReqSocket.Get();
            }
            bool isPost = Type == "POST";
            TSocketInput si(*s);

            if (UseHttpOutput) {
                TSocketOutput so(*s);
                THttpOutput output(&so);

                output.EnableKeepAlive(KeepAliveConnection);
                output.EnableCompression(EnableResponseEncoding);

                std::stringStream r;
                r << Type << " / HTTP/1.1" << CrLf;
                r << "Host: localhost:" + ToString(Port) << CrLf;
                if (isPost) {
                    if (ContentEncoding.size()) {
                        r << "Content-Encoding: " << ContentEncoding << CrLf;
                    } else {
                        r << "Transfer-Encoding: chunked" << CrLf;
                    }
                    if (Expect100Continue) {
                        r << "Expect: 100-continue" << CrLf;
                    }
                }

                r << CrLf;
                if (isPost) {
                    output.Write(r.Str());
                    output.Flush();
                    CheckContinue(si);
                    output.Write(Content);
                    output.Finish();
                } else {
                    output.Write(r.Str());
                    output.Finish();
                }
            } else {
                std::stringStream r;
                r << Type << " / HTTP/1.1" << CrLf;
                r << "Host: localhost:" + ToString(Port) << CrLf;
                if (KeepAliveConnection) {
                    r << "Connection: Keep-Alive" << CrLf;
                } else {
                    r << "Connection: Close" << CrLf;
                }
                if (EnableResponseEncoding) {
                    r << "Accept-Encoding: gzip, deflate, x-gzip, x-deflate, y-lzo, y-lzf, y-lzq, y-bzip2, y-lzma" << CrLf;
                }
                if (isPost && Expect100Continue) {
                    r << "Expect: 100-continue" << CrLf;
                }
                if (isPost && ContentEncoding.size() && Content.size()) {
                    r << "Content-Encoding: " << ContentEncoding << CrLf;
                    std::stringStream compressedContent;
                    {
                        TZLibCompress zlib(&compressedContent);
                        zlib.Write(Content.data(), Content.size());
                        zlib.Flush();
                        zlib.Finish();
                    }
                    r << "Content-Length: " << compressedContent.Size() << CrLf;
                    r << CrLf;
                    s->Send(r.Data(), r.Size());
                    CheckContinue(si);
                    Hdr = r.Str();
                    std::string tosend = compressedContent.Str();
                    s->Send(tosend.data(), tosend.size());
                } else {
                    if (isPost) {
                        r << "Content-Length: " << Content.size() << CrLf;
                        r << CrLf;
                        s->Send(r.Data(), r.Size());
                        CheckContinue(si);
                        Hdr = r.Str();
                        s->Send(Content.data(), Content.size());
                    } else {
                        r << CrLf;
                        Hdr = r.Str();
                        s->Send(r.Data(), r.Size());
                    }
                }
            }

            THttpInput input(&si);
            std::stringStream ss;
            TransferData(&input, &ss);

            return ss.Str();
        }

        std::string GetDescription() const {
            if (UseHttpOutput) {
                std::stringStream ss;
                ss << (KeepAliveConnection ? "keep-alive " : "") << Type;
                if (ContentEncoding.size()) {
                    ss << " with encoding=" << ContentEncoding;
                }
                return ss.Str();
            } else {
                return Hdr;
            }
        }

        ui16 Port = 0;
        bool UseHttpOutput = true;
        std::string Type = "GET";
        std::string ContentEncoding;
        std::string Content;
        bool KeepAliveConnection = false;
        THolder<TSocket> KeepAlivedSocket;
        bool EnableResponseEncoding = false;
        std::string Hdr;
        bool Expect100Continue = false;
    };

    class TFailingMtpQueue: public TSimpleThreadPool {
    private:
        bool FailOnAdd_ = false;

    public:
        void SetFailOnAdd(bool fail = true) {
            FailOnAdd_ = fail;
        }
        [[nodiscard]] bool Add(IObjectInQueue* pObj) override {
            if (FailOnAdd_) {
                return false;
            }

            return TSimpleThreadPool::Add(pObj);
        }
        TFailingMtpQueue() = default;
        TFailingMtpQueue(IThreadFactory* pool)
            : TSimpleThreadPool(pool)
        {
        }
    };

    std::string TestData(size_t size = 5 * 4096) {
        std::string res;

        for (size_t i = 0; i < size; ++i) {
            res += (char)i;
        }
        return res;
    }

    Y_UNIT_TEST(TestEchoServer) {
        std::string res = TestData();
        TPortManager pm;
        const ui16 port = pm.GetPort();
        const bool trueFalse[] = {true, false};

        TEchoServer serverImpl(res);
        THttpServer server(&serverImpl, THttpServer::TOptions(port).EnableKeepAlive(true).EnableCompression(true));

        for (int i = 0; i < 2; ++i) {
            UNIT_ASSERT(server.Start());

            TTestRequest r(port);
            r.Content = res;

            for (bool keepAlive : trueFalse) {
                r.KeepAliveConnection = keepAlive;

                // THttpOutput use chunked stream, else use Content-Length
                for (bool useHttpOutput : trueFalse) {
                    r.UseHttpOutput = useHttpOutput;

                    for (bool enableResponseEncoding : trueFalse) {
                        r.EnableResponseEncoding = enableResponseEncoding;

                        const std::string reqTypes[] = {"GET", "POST"};
                        for (const std::string& reqType : reqTypes) {
                            r.Type = reqType;

                            const std::string encoders[] = {"", "deflate"};
                            for (const std::string& encoder : encoders) {
                                r.ContentEncoding = encoder;

                                for (bool expect100Continue : trueFalse) {
                                    r.Expect100Continue = expect100Continue;
                                    std::string resp = r.Execute();
                                    UNIT_ASSERT_C(resp == res, "diff echo response for request:\n" + r.GetDescription());
                                }
                            }
                        }
                    }
                }
            }

            server.Stop();
        }
    }

    Y_UNIT_TEST(TestReusePortEnabled) {
        if (!IsReusePortAvailable()) {
            return; // skip test
        }
        std::string res = TestData();
        TPortManager pm;
        const ui16 port = pm.GetPort();

        TEchoServer serverImpl(res);
        std::vector<THolder<THttpServer>> servers;
        for (ui32 i = 0; i < 10; i++) {
            servers.push_back(MakeHolder<THttpServer>(&serverImpl, THttpServer::TOptions(port).EnableReusePort(true)));
        }

        for (ui32 testRun = 0; testRun < 3; testRun++) {
            for (auto& server : servers) {
                // start servers one at a time and check at least one of them is replying
                UNIT_ASSERT(server->Start());

                TTestRequest r(port, res);
                UNIT_ASSERT_C(r.Execute() == res, "diff echo response for request:\n" + r.GetDescription());
            }

            for (auto& server : servers) {
                // ping servers and stop them one at a time
                // at the last iteration only one server is still working and then gets stopped as well

                TTestRequest r(port, res);
                UNIT_ASSERT_C(r.Execute() == res, "diff echo response for request:\n" + r.GetDescription());

                server->Stop();
            }
        }
    }

    Y_UNIT_TEST(TestReusePortDisabled) {
        // check that with the ReusePort option disabled it's impossible to start two servers on the same port
        // check that ReusePort option is disabled by default (don't set it explicitly in the test)
        TPortManager pm;
        const ui16 port = pm.GetPort();

        TEchoServer serverImpl(std::string{});
        THttpServer server1(&serverImpl, THttpServer::TOptions(port));
        THttpServer server2(&serverImpl, THttpServer::TOptions(port));

        UNIT_ASSERT(true == server1.Start());
        UNIT_ASSERT(false == server2.Start());

        server1.Stop();
        // Stop() is a sync call, port should be free by now
        UNIT_ASSERT(true == server2.Start());
        UNIT_ASSERT(false == server1.Start());
    }

    Y_UNIT_TEST(TestFailServer) {
        /**
         * Emulate request processing failures
         * Data should be large enough not to fit into socket buffer
         **/
        std::string res = TestData(10 * 1024 * 1024);
        TPortManager portManager;
        const ui16 port = portManager.GetPort();
        TEchoServer serverImpl(res);
        THttpServer::TOptions options(port);
        options.EnableKeepAlive(true);
        options.EnableCompression(true);
        using TFailingServerMtpQueue = TThreadPoolBinder<TFailingMtpQueue, THttpServer::ICallBack>;
        THttpServer::TMtpQueueRef mainWorkers = new TFailingServerMtpQueue(&serverImpl, SystemThreadFactory());
        THttpServer::TMtpQueueRef failWorkers = new TThreadPool(SystemThreadFactory());
        THttpServer server(&serverImpl, mainWorkers, failWorkers, options);

        UNIT_ASSERT(server.Start());
        for (size_t i = 0; i < 3; ++i) {
            // should fail on 2nd request
            static_cast<TFailingMtpQueue*>(mainWorkers.Get())->SetFailOnAdd(i == 1);
            TTestRequest r(port);
            r.Content = res;
            r.Type = "POST";
            std::string resp = r.Execute();
            if (i == 1) {
                UNIT_ASSERT(resp.Contains("Service Unavailable"));
            } else {
                UNIT_ASSERT_C(resp == res, "diff echo response for request:\n" + r.GetDescription());
            }
        }
        server.Stop();
    }

    class TReleaseConnectionServer: public THttpServer::ICallBack {
        class TRequest: public THttpClientRequestEx {
        public:
            bool Reply(void* /*tsr*/) override {
                Output() << "HTTP/1.1 200 Ok\r\n\r\n";
                Output() << "reply";
                Output().Finish();

                ReleaseConnection();

                throw yexception() << "some error";

                return true;
            }
        };

    public:
        TClientRequest* CreateClient() override {
            return new TRequest();
        }

        void OnException() override {
            ExceptionMessage = CurrentExceptionMessage();
        }

        std::string ExceptionMessage;
    };

    class TResetConnectionServer: public THttpServer::ICallBack {
        class TRequest: public TClientRequest {
        public:
            bool Reply(void* /*tsr*/) override {
                Output() << "HTTP/1.1";
                ResetConnection();

                return true;
            }
        };

    public:
        TClientRequest* CreateClient() override {
            return new TRequest();
        }

        void OnException() override {
            ExceptionMessage = CurrentExceptionMessage();
        }

        std::string ExceptionMessage;
    };

    class TListenerSockAddrReplyServer: public THttpServer::ICallBack {
        class TRequest: public TClientRequest {
        public:
            bool Reply(void* /*tsr*/) override {
                Output() << "HTTP/1.1 200 Ok\r\n\r\n";
                Output() << PrintHostAndPort(*GetListenerSockAddrRef());

                Output().Finish();

                return true;
            }
        };

        public:
        TClientRequest* CreateClient() override {
            return new TRequest();
        }
    };

    Y_UNIT_TEST(TTestResetConnection) {
        TPortManager pm;
        const ui16 port = pm.GetPort();

        TResetConnectionServer serverImpl;
        THttpServer server(&serverImpl, THttpServer::TOptions(port));
        UNIT_ASSERT(server.Start());

        TTestRequest r(port, "request");

        UNIT_ASSERT_EXCEPTION_CONTAINS(r.Execute(), TSystemError, "Connection reset by peer");

        server.Stop();
    }

    Y_UNIT_TEST(TTestReleaseConnection) {
        TPortManager pm;
        const ui16 port = pm.GetPort();

        TReleaseConnectionServer serverImpl;
        THttpServer server(&serverImpl, THttpServer::TOptions(port).EnableKeepAlive(true));
        UNIT_ASSERT(server.Start());

        TTestRequest r(port, "request");
        r.KeepAliveConnection = true;

        UNIT_ASSERT_C(r.Execute() == "reply", "diff echo response for request:\n" + r.GetDescription());

        server.Stop();

        UNIT_ASSERT_STRINGS_EQUAL(serverImpl.ExceptionMessage, "(yexception) some error");
    }

    THttpInput SendRequest(TSocket& socket, ui16 port) {
        TSocketInput si(socket);
        TSocketOutput so(socket);
        THttpOutput out(&so);
        out.EnableKeepAlive(true);
        out << "GET / HTTP/1.1" << CrLf;
        out << "Host: localhost:" + ToString(port) << CrLf;
        out << CrLf;
        out.Flush();

        THttpInput input(&si);
        input.ReadAll();
        return input;
    }

    THttpInput SendRequestWithBody(TSocket& socket, ui16 port, std::string body) {
        TSocketInput si(socket);
        TSocketOutput so(socket);
        THttpOutput out(&so);
        out << "POST / HTTP/1.1" << CrLf;
        out << "Host: localhost:" + ToString(port) << CrLf;
        out << "Content-Length: " + ToString(body.size()) << CrLf;
        out << CrLf;
        out << body;
        out.Flush();

        THttpInput input(&si);
        input.ReadAll();
        return input;
    }

    Y_UNIT_TEST(TTestExpirationTimeout) {
        TPortManager pm;
        const ui16 port = pm.GetPort();

        TEchoServer serverImpl("test_data");
        THttpServer::TOptions options(port);
        options.nThreads = 1;
        options.MaxQueueSize = 0;
        options.MaxConnections = 0;
        options.KeepAliveEnabled = true;
        options.ExpirationTimeout = TDuration::Seconds(1);
        options.PollTimeout = TDuration::MilliSeconds(100);
        THttpServer server(&serverImpl, options);
        UNIT_ASSERT(server.Start());

        TSocket socket(TNetworkAddress("localhost", port), TDuration::Seconds(10));

        SendRequest(socket, port);
        SendRequest(socket, port);

        Sleep(TDuration::Seconds(5));
        UNIT_ASSERT_EXCEPTION(SendRequest(socket, port), THttpReadException);

        server.Stop();
    }

    Y_UNIT_TEST(TTestContentLengthTooLarge) {
        TPortManager pm;
        const ui16 port = pm.GetPort();

        TEchoServer serverImpl("test_data");
        THttpServer::TOptions options(port);
        options.nThreads = 1;
        options.MaxQueueSize = 0;
        options.MaxInputContentLength = 2_KB;
        options.MaxConnections = 0;
        options.KeepAliveEnabled = false;
        options.ExpirationTimeout = TDuration::Seconds(1);
        options.PollTimeout = TDuration::MilliSeconds(100);
        THttpServer server(&serverImpl, options);
        UNIT_ASSERT(server.Start());

        TSocket socket(TNetworkAddress("localhost", port), TDuration::Seconds(5));
        UNIT_ASSERT_STRING_CONTAINS(SendRequestWithBody(socket, port, std::string(1_KB, 'a')).FirstLine(), "HTTP/1.1 200 Ok");

        TSocket socket2(TNetworkAddress("localhost", port), TDuration::Seconds(5));
        UNIT_ASSERT_STRING_CONTAINS(SendRequestWithBody(socket2, port, std::string(10_KB, 'a')).FirstLine(), "HTTP/1.1 413 Payload Too Large");

        server.Stop();
    }

    Y_UNIT_TEST(TTestNullInRequest) {
        TPortManager pm;
        const ui16 port = pm.GetPort();

        TEchoServer serverImpl("test_data");
        THttpServer::TOptions options(port);
        options.nThreads = 1;
        options.MaxQueueSize = 0;
        options.MaxConnections = 0;
        options.KeepAliveEnabled = false;
        options.ExpirationTimeout = TDuration::Seconds(1);
        options.PollTimeout = TDuration::MilliSeconds(100);
        THttpServer server(&serverImpl, options);
        UNIT_ASSERT(server.Start());

        TSocket socket(TNetworkAddress("localhost", port), TDuration::Seconds(5));


        TSocketInput si(socket);
        TSocketOutput so(socket);
        THttpOutput out(&so);
        out << "GET \0/ggg HTTP/1.1" << CrLf;
        out << "Host: localhost:" + ToString(port) << CrLf;
        out << CrLf;
        out.Flush();

        THttpInput input(&si);
        input.ReadAll();

        UNIT_ASSERT_STRING_CONTAINS(input.FirstLine(), "HTTP/1.1 4");
        server.Stop();
    }


    Y_UNIT_TEST(TTestCloseConnectionOnRequestLimit) {
        TPortManager pm;
        const ui16 port = pm.GetPort();

        TEchoServer serverImpl("test_data");
        THttpServer server(&serverImpl, THttpServer::TOptions(port).EnableKeepAlive(true).SetMaxRequestsPerConnection(2));
        UNIT_ASSERT(server.Start());

        TSocket socket(TNetworkAddress("localhost", port), TDuration::Seconds(10));

        UNIT_ASSERT(SendRequest(socket, port).IsKeepAlive());
        UNIT_ASSERT(!SendRequest(socket, port).IsKeepAlive());

        UNIT_ASSERT_EXCEPTION(SendRequest(socket, port), THttpReadException);

        server.Stop();
    }

    Y_UNIT_TEST(TTestListenerSockAddrConnection) {
        TPortManager pm;
        const ui16 port1 = pm.GetPort();
        const ui16 port2 = pm.GetPort();

        TListenerSockAddrReplyServer serverImpl;
        THttpServer server(&serverImpl, THttpServer::TOptions().EnableKeepAlive(true).AddBindAddress("127.0.0.1", port1).AddBindAddress("127.0.0.1", port2));
        UNIT_ASSERT(server.Start());

        TTestRequest r1(port1);
        r1.KeepAliveConnection = true;

        std::string resp = r1.Execute();
        UNIT_ASSERT(resp == std::string::Join("127.0.0.1", ":", ToString(port1)));

        TTestRequest r2(port2);
        r2.KeepAliveConnection = true;

        resp = r2.Execute();
        UNIT_ASSERT(resp == std::string::Join("127.0.0.1", ":", ToString(port2)));

        server.Stop();
    }

    Y_UNIT_TEST(TestSocketsLeak) {
        TPortManager portManager;
        std::string res = TestData(25);

        const bool trueFalse[] = {true, false};

        for (bool rejectExcessConnections : trueFalse) {
            for (bool keepAlive : trueFalse) {
                const ui16 port = portManager.GetPort();
                TSleepingServer server;
                THttpServer::TOptions options(port);
                options.nThreads = 1;
                options.MaxConnections = 1;
                options.MaxQueueSize = 10;
                options.MaxFQueueSize = 2;
                options.nFThreads = 2;
                options.KeepAliveEnabled = true;
                options.RejectExcessConnections = rejectExcessConnections;
                THttpServer srv(&server, options);

                UNIT_ASSERT(srv.Start());
                UNIT_ASSERT(server.Lock.try_lock());

                std::atomic<size_t> threadsFinished = 0;
                std::vector<THolder<IThreadFactory::IThread>> threads;
                auto func = [port, keepAlive, &threadsFinished]() {
                    try {
                        TTestRequest r(port);
                        r.KeepAliveConnection = keepAlive;
                        r.Execute();
                    } catch (...) {
                    }
                    ++threadsFinished;
                };

                threads.push_back(SystemThreadFactory()->Run(func));

                while (server.Replies.load() != 1) { //wait while we have one connection inside server
                    Sleep(TDuration::MilliSeconds(1));
                }

                for (size_t i = 1; i < 3; ++i) {
                    threads.push_back(SystemThreadFactory()->Run(func));

                    //in case of rejectExcessConnections next requests will fail, otherwise will stuck inside server queue
                    while ((rejectExcessConnections ? threadsFinished.load() : srv.GetRequestQueueSize()) != i) {
                        Sleep(TDuration::MilliSeconds(1));
                    }
                }

                server.Lock.unlock();

                for (auto&& thread : threads) {
                    thread->Join();
                }

                std::stringStream opts;
                opts << " [" << rejectExcessConnections << ", " << keepAlive << "] ";

                UNIT_ASSERT_EQUAL_C(server.MaxConns, 2, opts.Str() +  "we should get MaxConn notification 2 times, got " + ToString(server.MaxConns.load()));
                if (rejectExcessConnections) {
                    UNIT_ASSERT_EQUAL_C(server.Replies, 1, opts.Str() + "only one request should have been processed, got " + ToString(server.Replies.load()));
                } else {
                    UNIT_ASSERT_VALUES_EQUAL(server.Replies.load(), 3);
                }
            }
        }
    }

    class TShooter {
    public:
        struct TCounters {
        public:
            TCounters() = default;
            TCounters(const TCounters& other)
                : Fail(other.Fail.load())
                , Success(other.Success.load())
            {
            }
        public:
            std::atomic<size_t> Fail = 0;
            std::atomic<size_t> Success = 0;
        };
    public:
        TShooter(size_t threadCount, ui16 port)
            : Counters_(threadCount)
        {
            for (size_t i = 0; i < threadCount; ++i) {
                auto func = [i, port, this] () {
                    for (;;) {
                        try {
                            TTestRequest r(port);
                            r.KeepAliveConnection = true;
                            for (size_t j = 0; j < 100; ++j) {
                                if (Stopped_.load()) {
                                    return;
                                }
                                r.Execute();
                                Sleep(TDuration::MilliSeconds(1) * RandomNumber<float>());
                                Counters_[i].Success++;
                            }
                        } catch (TSystemError& e) {
                            UNIT_ASSERT_C(e.Status() == ECONNRESET || e.Status() == ECONNREFUSED, CurrentExceptionMessage());
                            Counters_[i].Fail++;
                        } catch (THttpReadException&) {
                            Counters_[i].Fail++;
                        } catch (...) {
                            UNIT_ASSERT_C(false, CurrentExceptionMessage());
                        }
                    }
                };

                Threads_.push_back(SystemThreadFactory()->Run(func));
            }
        }

        void Stop() {
            Stopped_.store(true);
            for (auto& thread : Threads_) {
                thread->Join();
            }
        }

        void WaitProgress() const {
            auto snapshot = Counters_;
            for (;;) {
                size_t haveProgress = 0;
                for (size_t i = 0; i < Counters_.size(); ++i) {
                    haveProgress += (Counters_[i].Fail.load() + Counters_[i].Success.load()) > (snapshot[i].Fail + snapshot[i].Success);
                }

                if (haveProgress == Counters_.size()) {
                    return;
                }
                Sleep(TDuration::MilliSeconds(1));
            }
        }

        const auto& GetCounters() const {
            return Counters_;
        }

        ~TShooter() {
            Stop();
        }
    private:
        std::vector<THolder<IThreadFactory::IThread>> Threads_;
        std::atomic<bool> Stopped_ = false;
        std::vector<TCounters> Counters_;
    };

    struct TTestConfig {
        bool OneShot = false;
        ui32 ListenerThreads = 1;
    };

    std::vector<TTestConfig> testConfigs = {
        {.OneShot = false, .ListenerThreads = 1},
        {.OneShot = true, .ListenerThreads = 1},
        {.OneShot = true, .ListenerThreads = 4},
        {.OneShot = true, .ListenerThreads = 63},
    };

    THttpServer::TOptions ApplyConfig(const THttpServer::TOptions& opts, const TTestConfig& cfg) {
        THttpServer::TOptions res = opts;
        res.OneShotPoll = cfg.OneShot;
        res.nListenerThreads = cfg.ListenerThreads;
        return res;
    }

    Y_UNIT_TEST(TestStartStop) {
        TPortManager pm;
        const ui16 port = pm.GetPort();

        const size_t threadCount = 5;
        TShooter shooter(threadCount, port);

        std::string res = TestData();
        for (const auto& cfg : testConfigs) {
            TEchoServer serverImpl(res);
            THttpServer server(&serverImpl, ApplyConfig(THttpServer::TOptions(port).EnableKeepAlive(true), cfg));
            for (size_t i = 0; i < 100; ++i) {
                UNIT_ASSERT(server.Start());
                shooter.WaitProgress();

                {
                    auto before = shooter.GetCounters();
                    shooter.WaitProgress();
                    auto after = shooter.GetCounters();
                    for (size_t i = 0; i < before.size(); ++i) {
                        UNIT_ASSERT(before[i].Success < after[i].Success);
                        UNIT_ASSERT(before[i].Fail == after[i].Fail);
                    }
                }

                server.Stop();
                shooter.WaitProgress();
                {
                    auto before = shooter.GetCounters();
                    shooter.WaitProgress();
                    auto after = shooter.GetCounters();
                    for (size_t i = 0; i < before.size(); ++i) {
                        UNIT_ASSERT(before[i].Success == after[i].Success);
                        UNIT_ASSERT(before[i].Fail < after[i].Fail);
                    }
                }
            }
        }
    }

    Y_UNIT_TEST(TestMaxConnections) {
        class TMaxConnServer
            : public TEchoServer
        {
        public:
            using TEchoServer::TEchoServer;

            void OnMaxConn() override {
                ++MaxConns;
            }
        public:
            std::atomic<size_t> MaxConns = 0;

        };

        TPortManager pm;
        const ui16 port = pm.GetPort();

        const size_t maxConnections = 5;

        std::string res = TestData();

        for (const auto& cfg : testConfigs) {
            TMaxConnServer serverImpl(res);
            THttpServer server(&serverImpl, ApplyConfig(THttpServer::TOptions(port).EnableKeepAlive(true).SetMaxConnections(maxConnections), cfg));

            UNIT_ASSERT(server.Start());

            TShooter shooter(maxConnections + 1, port);

            for (size_t i = 0; i < 100; ++i) {
                const size_t prev = serverImpl.MaxConns.load();
                while (serverImpl.MaxConns.load() < prev + 100) {
                    Sleep(TDuration::MilliSeconds(1));
                }
            }

            shooter.Stop();
            server.Stop();

            for (const auto& c : shooter.GetCounters()) {
                UNIT_ASSERT(c.Success > 0);
                UNIT_ASSERT(c.Fail > 0);
                UNIT_ASSERT(c.Success > c.Fail);
            }
        }
    }

    Y_UNIT_TEST(StartFail) {
        std::string res = TestData();
        TEchoServer serverImpl(res);
        {
            THttpServer server(&serverImpl, THttpServer::TOptions(1));

            UNIT_ASSERT(!server.GetErrorCode());
            UNIT_ASSERT(!server.Start());
            UNIT_ASSERT(server.GetErrorCode());
        }

        {
            TPortManager pm;
            const ui16 port = pm.GetPort();
            THttpServer server1(&serverImpl, THttpServer::TOptions(port));
            UNIT_ASSERT(server1.Start());
            UNIT_ASSERT(!server1.GetErrorCode());

            THttpServer server2(&serverImpl, THttpServer::TOptions(port));
            UNIT_ASSERT(!server2.Start());
            UNIT_ASSERT(server2.GetErrorCode());
        }

    }

    inline std::string ToString(const std::unordered_set<std::string>& hs) {
        std::string res = "";
        for (auto s : hs) {
            if (res) {
                res.append(",");
            }
            res.append("\"").append(s).append("\"");
        }
        return res;
    }

    Y_UNIT_TEST(TestTTLExceed) {
        // Checks that one of request returns "TTL Exceed"
        // First request waits for server.Lock.unlock() for one threaded TSleepingServer
        // So second request in queue should fail with TTL Exceed, because fist one lock thread pool for (ttl + 1) ms
        TPortManager portManager;
        const ui16 port = portManager.GetPort();
        std::string res = TestData(25);
        const size_t ttl = 10;
        TSleepingServer server{ttl};
        THttpServer::TOptions options(port);
        options.nThreads = 1;
        options.MaxConnections = 2;
        THttpServer srv(&server, options);

        UNIT_ASSERT(srv.Start());
        UNIT_ASSERT(server.Lock.try_lock());

        std::unordered_set<std::string> results;
        std::mutex resultLock;
        auto func = [port, &resultLock, &results]() {
            try {
                TTestRequest r(port);
                std::string result = r.Execute();
                std::lock_guard guard(resultLock);
                results.insert(result);
            } catch (...) {
            }
        };

        auto t1 = SystemThreadFactory()->Run(func);
        auto t2 = SystemThreadFactory()->Run(func);
        Sleep(TDuration::MilliSeconds(ttl + 1));
        server.Lock.unlock();
        t1->Join();
        t2->Join();
        UNIT_ASSERT_EQUAL_C(results, (std::unordered_set<std::string>({"Zoooo", "TTL Exceed"})), "Results is {" + ToString(results) + "}");
    }
}
