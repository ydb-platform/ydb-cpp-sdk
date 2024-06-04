#include "md5.h"

#include "registar.h"

Y_UNIT_TEST_SUITE(TMD5MediumTest) {
    Y_UNIT_TEST(TestOverflow) {
        if (sizeof(size_t) > sizeof(unsigned int)) {
            const size_t maxUi32 = static_cast<size_t>(Max<unsigned int>());
            auto buf = std::make_unique<char[]>(maxUi32);

            memset(buf.get(), 0, maxUi32);

            MD5 r;
            for (int i = 0; i < 5; ++i) {
                r.Update(buf.get(), maxUi32);
            }

            char rs[33];
            std::string s(r.End(rs));
            std::ranges::transform(s, s.begin(), ::tolower);

            UNIT_ASSERT_VALUES_EQUAL(s, "34a5a7ed4f0221310084e21a1e599659");
        }
    }
}
