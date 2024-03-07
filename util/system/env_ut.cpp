#include <library/cpp/testing/unittest/registar.h>

#include "env.h"

Y_UNIT_TEST_SUITE(EnvTest) {
    Y_UNIT_TEST(GetSetEnvTest) {
        std::string key = "util_GETENV_TestVar";
        std::string value = "Some value for env var";
        std::string def = "Some default value for env var";
        // first of all, it should be clear
        UNIT_ASSERT_VALUES_EQUAL(GetEnv(key), std::string());
        UNIT_ASSERT_VALUES_EQUAL(GetEnv(key, def), def);
        SetEnv(key, value);
        // set and see what value we get here
        UNIT_ASSERT_VALUES_EQUAL(GetEnv(key), value);
        UNIT_ASSERT_VALUES_EQUAL(GetEnv(key, def), value);
        // set empty value
        SetEnv(key, std::string());
        UNIT_ASSERT_VALUES_EQUAL(GetEnv(key), std::string());

        // check for long values, see IGNIETFERRO-214
        std::string longKey = "util_GETENV_TestVarLong";
        std::string longValue{1500, 't'};
        UNIT_ASSERT_VALUES_EQUAL(GetEnv(longKey), std::string());
        SetEnv(longKey, longValue);
        UNIT_ASSERT_VALUES_EQUAL(GetEnv(longKey), longValue);
        SetEnv(longKey, std::string());
        UNIT_ASSERT_VALUES_EQUAL(GetEnv(longKey), std::string());
    }
}
