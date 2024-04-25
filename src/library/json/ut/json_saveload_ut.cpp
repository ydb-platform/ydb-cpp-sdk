<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/library/json/json_value.h>
=======
#include <src/library/json/json_value.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/library/json/json_value.h>
>>>>>>> origin/main

#include <src/library/testing/unittest/registar.h>
#include <src/util/stream/buffer.h>
#include <src/util/generic/buffer.h>
<<<<<<< HEAD
<<<<<<< HEAD
#include <ydb-cpp-sdk/util/ysaveload.h>
=======
#include <src/util/ysaveload.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
#include <ydb-cpp-sdk/util/ysaveload.h>
>>>>>>> origin/main

Y_UNIT_TEST_SUITE(JsonSaveLoad) {
    Y_UNIT_TEST(Serialize) {

        NJson::TJsonValue expected;

        expected["ui64"] = ui64(1);
        expected["i64"] = i64(2);
        expected["double"] = 2.0;
        expected["string"] = "text";
        expected["map"] = expected;
        expected["array"].SetType(NJson::JSON_ARRAY).GetArraySafe().emplace_back(expected);
        expected["null"].SetType(NJson::JSON_NULL);
        expected["undefined"].SetType(NJson::JSON_UNDEFINED);

        TBuffer buffer;
        {
            TBufferOutput output(buffer);
            ::Save(&output, expected);
        }

        NJson::TJsonValue load;
        {
            TBufferInput input(buffer);
            ::Load(&input, load);
        }

        UNIT_ASSERT_EQUAL_C(expected, load, "expected: " << expected << ", got: " << load);
    }
}
