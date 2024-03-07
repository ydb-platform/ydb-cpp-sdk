#include "hex.h"

#include <library/cpp/testing/unittest/registar.h>
#include "str.h"

Y_UNIT_TEST_SUITE(THexCodingTest) {
    void TestImpl(const std::string& data) {
        std::string encoded;
        TStringOutput encodedOut(encoded);
        HexEncode(data.data(), data.size(), encodedOut);

        UNIT_ASSERT_EQUAL(encoded.size(), data.size() * 2);

        std::string decoded;
        TStringOutput decodedOut(decoded);
        HexDecode(encoded.data(), encoded.size(), decodedOut);

        UNIT_ASSERT_EQUAL(decoded, data);
    }

    Y_UNIT_TEST(TestEncodeDecodeToStream) {
        std::string data = "100ABAcaba500,$%0987123456   \n\t\x01\x02\x03.";
        TestImpl(data);
    }

    Y_UNIT_TEST(TestEmpty) {
        TestImpl("");
    }
}
