#include "json_value.h"

#include "registar.h"
#include "buffer.h"
#include <src/util/generic/buffer.h>
#include "ysaveload.h"

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
