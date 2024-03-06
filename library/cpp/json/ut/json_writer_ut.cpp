#include <library/cpp/json/json_writer.h>
#include <library/cpp/testing/unittest/registar.h>

#include <util/stream/str.h>

using namespace NJson;

Y_UNIT_TEST_SUITE(TJsonWriterTest) {
    Y_UNIT_TEST(SimpleWriteTest) {
        std::string expected1 = "{\"key1\":1,\"key2\":2,\"key3\":3";
        std::string expected2 = expected1 + ",\"array\":[\"stroka\",false]";
        std::string expected3 = expected2 + "}";

        std::stringStream out;

        TJsonWriter json(&out, false);
        json.OpenMap();
        json.Write("key1", (ui16)1);
        json.WriteKey("key2");
        json.Write((i32)2);
        json.Write("key3", (ui64)3);

        UNIT_ASSERT(out.Empty());
        json.Flush();
        UNIT_ASSERT_VALUES_EQUAL(out.Str(), expected1);

        json.Write("array");
        json.OpenArray();
        json.Write("stroka");
        json.Write(false);
        json.CloseArray();

        UNIT_ASSERT_VALUES_EQUAL(out.Str(), expected1);
        json.Flush();
        UNIT_ASSERT_VALUES_EQUAL(out.Str(), expected2);

        json.CloseMap();

        UNIT_ASSERT_VALUES_EQUAL(out.Str(), expected2);
        json.Flush();
        UNIT_ASSERT_VALUES_EQUAL(out.Str(), expected3);
    }

    Y_UNIT_TEST(SimpleWriteValueTest) {
        std::string expected = "{\"key1\":null,\"key2\":{\"subkey1\":[1,{\"subsubkey\":\"test2\"},null,true],\"subkey2\":\"test\"}}";
        TJsonValue v;
        v["key1"] = JSON_NULL;
        v["key2"]["subkey1"].AppendValue(1);
        v["key2"]["subkey1"].AppendValue(JSON_MAP)["subsubkey"] = "test2";
        v["key2"]["subkey1"].AppendValue(JSON_NULL);
        v["key2"]["subkey1"].AppendValue(true);
        v["key2"]["subkey2"] = "test";
        std::stringStream out;
        WriteJson(&out, &v);
        UNIT_ASSERT_VALUES_EQUAL(out.Str(), expected);
    }

    Y_UNIT_TEST(FormatOutput) {
        std::string expected = "{\n  \"key1\":null,\n  \"key2\":\n    {\n      \"subkey1\":\n        [\n          1,\n          {\n            \"subsubkey\":\"test2\"\n          },\n          null,\n          true\n        ],\n      \"subkey2\":\"test\"\n    }\n}";
        TJsonValue v;
        v["key1"] = JSON_NULL;
        v["key2"]["subkey1"].AppendValue(1);
        v["key2"]["subkey1"].AppendValue(JSON_MAP)["subsubkey"] = "test2";
        v["key2"]["subkey1"].AppendValue(JSON_NULL);
        v["key2"]["subkey1"].AppendValue(true);
        v["key2"]["subkey2"] = "test";
        std::stringStream out;
        WriteJson(&out, &v, true);
        UNIT_ASSERT_STRINGS_EQUAL(out.Str(), expected);
    }

    Y_UNIT_TEST(SortKeys) {
        std::string expected = "{\"a\":null,\"j\":null,\"n\":null,\"y\":null,\"z\":null}";
        TJsonValue v;
        v["z"] = JSON_NULL;
        v["n"] = JSON_NULL;
        v["a"] = JSON_NULL;
        v["y"] = JSON_NULL;
        v["j"] = JSON_NULL;
        std::stringStream out;
        WriteJson(&out, &v, false, true);
        UNIT_ASSERT_STRINGS_EQUAL(out.Str(), expected);
    }

    Y_UNIT_TEST(SimpleUnsignedIntegerWriteTest) {
        {
            std::string expected = "{\"test\":1}";
            TJsonValue v;
            v.InsertValue("test", 1ull);
            std::stringStream out;
            WriteJson(&out, &v);
            UNIT_ASSERT_VALUES_EQUAL(out.Str(), expected);
        } // 1

        {
            std::string expected = "{\"test\":-1}";
            TJsonValue v;
            v.InsertValue("test", -1);
            std::stringStream out;
            WriteJson(&out, &v);
            UNIT_ASSERT_VALUES_EQUAL(out.Str(), expected);
        } // -1

        {
            std::string expected = "{\"test\":18446744073709551615}";
            TJsonValue v;
            v.InsertValue("test", 18446744073709551615ull);
            std::stringStream out;
            WriteJson(&out, &v);
            UNIT_ASSERT_VALUES_EQUAL(out.Str(), expected);
        } // 18446744073709551615

        {
            std::string expected = "{\"test\":[1,18446744073709551615]}";
            TJsonValue v;
            v.InsertValue("test", TJsonValue());
            v["test"].AppendValue(1);
            v["test"].AppendValue(18446744073709551615ull);
            std::stringStream out;
            WriteJson(&out, &v);
            UNIT_ASSERT_VALUES_EQUAL(out.Str(), expected);
        } // 18446744073709551615
    }     // SimpleUnsignedIntegerWriteTest

    Y_UNIT_TEST(WriteOptionalTest) {
        {
            std::string expected = "{\"test\":1}";

            std::stringStream out;

            TJsonWriter json(&out, false);
            json.OpenMap();
            json.WriteOptional("test", MakeMaybe<int>(1));
            json.CloseMap();
            json.Flush();
            UNIT_ASSERT_VALUES_EQUAL(out.Str(), expected);
        }

        {
            std::string expected = "{}";

            std::stringStream out;

            std::optional<int> nothing = std::nullopt;

            TJsonWriter json(&out, false);
            json.OpenMap();
            json.WriteOptional("test", nothing);
            json.CloseMap();
            json.Flush();
            UNIT_ASSERT_VALUES_EQUAL(out.Str(), expected);
        }

        {
            std::string expected = "{}";

            std::stringStream out;

            std::optional<int> empty;

            TJsonWriter json(&out, false);
            json.OpenMap();
            json.WriteOptional("test", empty);
            json.CloseMap();
            json.Flush();
            UNIT_ASSERT_VALUES_EQUAL(out.Str(), expected);
        }

        {
            std::string expected = "{}";

            std::stringStream out;

            TJsonWriter json(&out, false);
            json.OpenMap();
            json.WriteOptional("test", Nothing());
            json.CloseMap();
            json.Flush();
            UNIT_ASSERT_VALUES_EQUAL(out.Str(), expected);
        }
    }

    Y_UNIT_TEST(Callback) {
        NJsonWriter::TBuf json;
        json.WriteString("A");
        UNIT_ASSERT_VALUES_EQUAL(json.Str(), "\"A\"");
        UNIT_ASSERT_VALUES_EQUAL(WrapJsonToCallback(json, ""), "\"A\"");
        UNIT_ASSERT_VALUES_EQUAL(WrapJsonToCallback(json, "Foo"), "Foo(\"A\")");
    }

    Y_UNIT_TEST(FloatPrecision) {
        const double value = 1517933989.4242;
        const NJson::TJsonValue json(value);
        NJson::TJsonWriterConfig config;
        {
            std::string expected = "1517933989";
            std::string actual = NJson::WriteJson(json);
            UNIT_ASSERT_VALUES_EQUAL(actual, expected);
        }
        {
            std::string expected = "1517933989";

            std::stringStream ss;
            NJson::WriteJson(&ss, &json, config);
            std::string actual = ss.Str();
            UNIT_ASSERT_VALUES_EQUAL(actual, expected);
        }
        {
            config.DoubleNDigits = 13;
            std::string expected = "1517933989.424";

            std::stringStream ss;
            NJson::WriteJson(&ss, &json, config);
            std::string actual = ss.Str();
            UNIT_ASSERT_VALUES_EQUAL(actual, expected);
        }
        {
            config.DoubleNDigits = 6;
            config.FloatToStringMode = PREC_POINT_DIGITS;
            std::string expected = "1517933989.424200";

            std::stringStream ss;
            NJson::WriteJson(&ss, &json, config);
            std::string actual = ss.Str();
            UNIT_ASSERT_VALUES_EQUAL(actual, expected);
        }
    }
}
