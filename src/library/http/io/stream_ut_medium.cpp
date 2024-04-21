#include "stream.h"
#include <src/library/testing/unittest/registar.h>
#include <src/util/stream/zlib.h>

Y_UNIT_TEST_SUITE(THttpTestMedium) {
    Y_UNIT_TEST(TestCodings2) {
        std::string_view data = "aaaaaaaaaaaaaaaaaaaaaaa";

        for (auto codec : SupportedCodings()) {
            if (codec == std::string_view("z-zlib-0")) {
                continue;
            }

            if (codec == std::string_view("z-null")) {
                continue;
            }

            std::string s;

            {
                TStringOutput so(s);
                THttpOutput ho(&so);
                TBufferedOutput bo(&ho, 10000);

                bo << "HTTP/1.1 200 Ok\r\n"
                   << "Connection: close\r\n"
                   << "Content-Encoding: " << codec << "\r\n\r\n";

                for (size_t i = 0; i < 100; ++i) {
                    bo << data;
                }
            }

            try {
                UNIT_ASSERT(s.size() > 10);
                UNIT_ASSERT(s.find(data) == std::string::npos);
            } catch (...) {
                std::cerr << codec << " " << s << std::endl;

                throw;
            }

            {
                TStringInput si(s);
                THttpInput hi(&si);

                auto res = hi.ReadAll();

                UNIT_ASSERT(res.find(data) == 0);
            }
        }
    }

} // THttpTestMedium suite
