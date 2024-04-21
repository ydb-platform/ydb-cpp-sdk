#include <src/library/openssl/io/stream.h>
#include <src/library/testing/unittest/registar.h>

Y_UNIT_TEST_SUITE(Builtin) {
    Y_UNIT_TEST(Init) {
        UNIT_ASSERT_NO_EXCEPTION(GetBuiltinOpenSslX509Store());
        UNIT_ASSERT_NO_EXCEPTION(GetBuiltinOpenSslX509Store());
    }
}
