#include <src/library/http/simple/http_client.h>

<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/library/http/server/response.h>
=======
#include <src/library/http/server/response.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/library/http/server/response.h>
>>>>>>> origin/main

#include <src/library/testing/unittest/registar.h>
#include <src/library/testing/unittest/tests_data.h>

#include <src/util/system/shellcommand.h>

#include <iostream>

Y_UNIT_TEST_SUITE(Https) {
    using TShellCommandPtr = std::unique_ptr<TShellCommand>;

    static TShellCommandPtr start(ui16 port) {
        const std::string data = ArcadiaSourceRoot() + "/src/library/http/simple/ut/https_server";

        const std::string command =
            TStringBuilder()
            << BuildRoot() << "/src/library/http/simple/ut/https_server/https_server"
            << " --port " << port
            << " --keyfile " << data << "/http_server.key"
            << " --certfile " << data << "/http_server.crt";

        auto res = std::make_unique<TShellCommand>(
            command,
            TShellCommandOptions()
                .SetAsync(true)
                .SetLatency(50)
                .SetErrorStream(&Cerr));

        res->Run();

        i32 tries = 100000;
        while (tries-- > 0) {
            try {
                TKeepAliveHttpClient client("https://localhost", port);
                client.DisableVerificationForHttps();
                client.DoGet("/ping");
                break;
            } catch (const std::exception& e) {
                std::cout << "== failed to connect to new server: " << e.what() << std::endl;
                Sleep(TDuration::MilliSeconds(1));
            }
        }

        return res;
    }

    static void get(TKeepAliveHttpClient & client) {
        TStringStream out;
        ui32 code = 0;

        UNIT_ASSERT_NO_EXCEPTION(code = client.DoGet("/ping", &out));
        UNIT_ASSERT_VALUES_EQUAL_C(code, 200, out.Str());
        UNIT_ASSERT_VALUES_EQUAL(out.Str(), "pong.my");
    }

    Y_UNIT_TEST(keepAlive) {
        TPortManager pm;
        ui16 port = pm.GetPort(443);
        TShellCommandPtr httpsServer = start(port);

        TKeepAliveHttpClient client("https://localhost",
                                    port,
                                    TDuration::Seconds(40),
                                    TDuration::Seconds(40));
        client.DisableVerificationForHttps();

        get(client);
        get(client);

        httpsServer->Terminate().Wait();
        httpsServer = start(port);

        get(client);
    }

    static void get(TSimpleHttpClient & client) {
        TStringStream out;

        UNIT_ASSERT_NO_EXCEPTION_C(client.DoGet("/ping", &out), out.Str());
        UNIT_ASSERT_VALUES_EQUAL(out.Str(), "pong.my");
    }

    Y_UNIT_TEST(simple) {
        TPortManager pm;
        ui16 port = pm.GetPort(443);
        TShellCommandPtr httpsServer = start(port);

        TSimpleHttpClient client("https://localhost",
                                 port,
                                 TDuration::Seconds(40),
                                 TDuration::Seconds(40));

        get(client);
        get(client);
    }
}
