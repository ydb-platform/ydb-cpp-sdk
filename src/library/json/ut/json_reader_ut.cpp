#include <src/library/json/json_reader.h>
#include <src/library/json/json_writer.h>

#include <src/library/testing/unittest/registar.h>
#include <src/util/stream/str.h>

using namespace NJson;

class TReformatCallbacks: public TJsonCallbacks {
    TJsonWriter& Writer;

public:
    TReformatCallbacks(TJsonWriter& writer)
        : Writer(writer)
    {
    }

    bool OnBoolean(bool val) override {
        Writer.Write(val);
        return true;
    }

    bool OnInteger(long long val) override {
        Writer.Write(val);
        return true;
    }

    bool OnUInteger(unsigned long long val) override {
        Writer.Write(val);
        return true;
    }

    bool OnString(const std::string_view& val) override {
        Writer.Write(val);
        return true;
    }

    bool OnDouble(double val) override {
        Writer.Write(val);
        return true;
    }

    bool OnOpenArray() override {
        Writer.OpenArray();
        return true;
    }

    bool OnCloseArray() override {
        Writer.CloseArray();
        return true;
    }

    bool OnOpenMap() override {
        Writer.OpenArray();
        return true;
    }

    bool OnCloseMap() override {
        Writer.CloseArray();
        return true;
    }

    bool OnMapKey(const std::string_view& val) override {
        Writer.Write(val);
        return true;
    }
};

void GenerateDeepJson(std::stringStream& stream, ui64 depth) {
    stream << "{\"key\":";
    for (ui32 i = 0; i < depth - 1; ++i) {
        stream << "[";
    }
    for (ui32 i = 0; i < depth - 1; ++i) {
        stream << "]";
    }
    stream << "}";
}

Y_UNIT_TEST_SUITE(TJsonReaderTest) {
    Y_UNIT_TEST(JsonReformatTest) {
        std::string data = "{\"null value\": null, \"intkey\": 10, \"double key\": 11.11, \"string key\": \"string\", \"array\": [1,2,3,\"std::string\"], \"bool key\": true}";

        std::string result1, result2;
        {
            std::stringStream in;
            in << data;
            std::stringStream out;
            TJsonWriter writer(&out, false);
            TReformatCallbacks cb(writer);
            ReadJson(&in, &cb);
            writer.Flush();
            result1 = out.Str();
        }

        {
            std::stringStream in;
            in << result1;
            std::stringStream out;
            TJsonWriter writer(&out, false);
            TReformatCallbacks cb(writer);
            ReadJson(&in, &cb);
            writer.Flush();
            result2 = out.Str();
        }

        UNIT_ASSERT_VALUES_EQUAL(result1, result2);
    }

    Y_UNIT_TEST(TJsonEscapedApostrophe) {
        std::string jsonString = "{ \"foo\" : \"bar\\'buzz\" }";
        {
            std::stringStream in;
            in << jsonString;
            std::stringStream out;
            TJsonWriter writer(&out, false);
            TReformatCallbacks cb(writer);
            UNIT_ASSERT(!ReadJson(&in, &cb));
        }

        {
            std::stringStream in;
            in << jsonString;
            std::stringStream out;
            TJsonWriter writer(&out, false);
            TReformatCallbacks cb(writer);
            UNIT_ASSERT(ReadJson(&in, false, true, &cb));
            writer.Flush();
            UNIT_ASSERT_EQUAL(out.Str(), "[\"foo\",\"bar'buzz\"]");
        }
    }

    Y_UNIT_TEST(TJsonTreeTest) {
        std::string data = "{\"intkey\": 10, \"double key\": 11.11, \"null value\":null, \"string key\": \"string\", \"array\": [1,2,3,\"std::string\"], \"bool key\": true}";
        std::stringStream in;
        in << data;
        TJsonValue value;
        ReadJsonTree(&in, &value);

        UNIT_ASSERT_VALUES_EQUAL(value["intkey"].GetInteger(), 10);
        UNIT_ASSERT_DOUBLES_EQUAL(value["double key"].GetDouble(), 11.11, 0.001);
        UNIT_ASSERT_VALUES_EQUAL(value["bool key"].GetBoolean(), true);
        UNIT_ASSERT_VALUES_EQUAL(value["absent string key"].GetString(), std::string(""));
        UNIT_ASSERT_VALUES_EQUAL(value["string key"].GetString(), std::string("string"));
        UNIT_ASSERT_VALUES_EQUAL(value["array"][0].GetInteger(), 1);
        UNIT_ASSERT_VALUES_EQUAL(value["array"][1].GetInteger(), 2);
        UNIT_ASSERT_VALUES_EQUAL(value["array"][2].GetInteger(), 3);
        UNIT_ASSERT_VALUES_EQUAL(value["array"][3].GetString(), std::string("std::string"));
        UNIT_ASSERT(value["null value"].IsNull());

        // AsString
        UNIT_ASSERT_VALUES_EQUAL(value["intkey"].GetStringRobust(), "10");
        UNIT_ASSERT_VALUES_EQUAL(value["double key"].GetStringRobust(), "11.11");
        UNIT_ASSERT_VALUES_EQUAL(value["bool key"].GetStringRobust(), "true");
        UNIT_ASSERT_VALUES_EQUAL(value["string key"].GetStringRobust(), "string");
        UNIT_ASSERT_VALUES_EQUAL(value["array"].GetStringRobust(), "[1,2,3,\"std::string\"]");
        UNIT_ASSERT_VALUES_EQUAL(value["null value"].GetStringRobust(), "null");

        const TJsonValue::TArray* array;
        UNIT_ASSERT(GetArrayPointer(value, "array", &array));
        UNIT_ASSERT_VALUES_EQUAL(value["array"].GetArray().size(), array->size());
        UNIT_ASSERT_VALUES_EQUAL(value["array"][0].GetInteger(), (*array)[0].GetInteger());
        UNIT_ASSERT_VALUES_EQUAL(value["array"][1].GetInteger(), (*array)[1].GetInteger());
        UNIT_ASSERT_VALUES_EQUAL(value["array"][2].GetInteger(), (*array)[2].GetInteger());
        UNIT_ASSERT_VALUES_EQUAL(value["array"][3].GetString(), (*array)[3].GetString());
    }

    Y_UNIT_TEST(TJsonRomaTest) {
        std::string data = "{\"test\": [ {\"name\": \"A\"} ]}";

        std::stringStream in;
        in << data;
        TJsonValue value;
        ReadJsonTree(&in, &value);

        UNIT_ASSERT_VALUES_EQUAL(value["test"][0]["name"].GetString(), std::string("A"));
    }

    Y_UNIT_TEST(TJsonReadTreeWithComments) {
        {
            std::string leadingCommentData = "{ // \"test\" : 1 \n}";
            {
                // No comments allowed
                std::stringStream in;
                in << leadingCommentData;
                TJsonValue value;
                UNIT_ASSERT(!ReadJsonTree(&in, false, &value));
            }

            {
                // Comments allowed
                std::stringStream in;
                in << leadingCommentData;
                TJsonValue value;
                UNIT_ASSERT(ReadJsonTree(&in, true, &value));
                UNIT_ASSERT(!value.Has("test"));
            }
        }

        {
            std::string trailingCommentData = "{ \"test1\" : 1 // \"test2\" : 2 \n }";
            {
                // No comments allowed
                std::stringStream in;
                in << trailingCommentData;
                TJsonValue value;
                UNIT_ASSERT(!ReadJsonTree(&in, false, &value));
            }

            {
                // Comments allowed
                std::stringStream in;
                in << trailingCommentData;
                TJsonValue value;
                UNIT_ASSERT(ReadJsonTree(&in, true, &value));
                UNIT_ASSERT(value.Has("test1"));
                UNIT_ASSERT_EQUAL(value["test1"].GetInteger(), 1);
                UNIT_ASSERT(!value.Has("test2"));
            }
        }
    }

    Y_UNIT_TEST(TJsonSignedIntegerTest) {
        {
            std::stringStream in;
            in << "{ \"test\" : " << Min<i64>() << " }";
            TJsonValue value;
            UNIT_ASSERT(ReadJsonTree(&in, &value));
            UNIT_ASSERT(value.Has("test"));
            UNIT_ASSERT(value["test"].IsInteger());
            UNIT_ASSERT(!value["test"].IsUInteger());
            UNIT_ASSERT_EQUAL(value["test"].GetInteger(), Min<i64>());
            UNIT_ASSERT_EQUAL(value["test"].GetIntegerRobust(), Min<i64>());
        } // Min<i64>()

        {
            std::stringStream in;
            in << "{ \"test\" : " << Max<i64>() + 1ull << " }";
            TJsonValue value;
            UNIT_ASSERT(ReadJsonTree(&in, &value));
            UNIT_ASSERT(value.Has("test"));
            UNIT_ASSERT(!value["test"].IsInteger());
            UNIT_ASSERT(value["test"].IsUInteger());
            UNIT_ASSERT_EQUAL(value["test"].GetIntegerRobust(), (i64)(Max<i64>() + 1ull));
        } // Max<i64>() + 1
    }

    Y_UNIT_TEST(TJsonUnsignedIntegerTest) {
        {
            std::stringStream in;
            in << "{ \"test\" : 1 }";
            TJsonValue value;
            UNIT_ASSERT(ReadJsonTree(&in, &value));
            UNIT_ASSERT(value.Has("test"));
            UNIT_ASSERT(value["test"].IsInteger());
            UNIT_ASSERT(value["test"].IsUInteger());
            UNIT_ASSERT_EQUAL(value["test"].GetInteger(), 1);
            UNIT_ASSERT_EQUAL(value["test"].GetIntegerRobust(), 1);
            UNIT_ASSERT_EQUAL(value["test"].GetUInteger(), 1);
            UNIT_ASSERT_EQUAL(value["test"].GetUIntegerRobust(), 1);
        } // 1

        {
            std::stringStream in;
            in << "{ \"test\" : -1 }";
            TJsonValue value;
            UNIT_ASSERT(ReadJsonTree(&in, &value));
            UNIT_ASSERT(value.Has("test"));
            UNIT_ASSERT(value["test"].IsInteger());
            UNIT_ASSERT(!value["test"].IsUInteger());
            UNIT_ASSERT_EQUAL(value["test"].GetInteger(), -1);
            UNIT_ASSERT_EQUAL(value["test"].GetIntegerRobust(), -1);
            UNIT_ASSERT_EQUAL(value["test"].GetUInteger(), 0);
            UNIT_ASSERT_EQUAL(value["test"].GetUIntegerRobust(), static_cast<unsigned long long>(-1));
        } // -1

        {
            std::stringStream in;
            in << "{ \"test\" : 18446744073709551615 }";
            TJsonValue value;
            UNIT_ASSERT(ReadJsonTree(&in, &value));
            UNIT_ASSERT(value.Has("test"));
            UNIT_ASSERT(!value["test"].IsInteger());
            UNIT_ASSERT(value["test"].IsUInteger());
            UNIT_ASSERT_EQUAL(value["test"].GetInteger(), 0);
            UNIT_ASSERT_EQUAL(value["test"].GetIntegerRobust(), static_cast<long long>(18446744073709551615ull));
            UNIT_ASSERT_EQUAL(value["test"].GetUInteger(), 18446744073709551615ull);
            UNIT_ASSERT_EQUAL(value["test"].GetUIntegerRobust(), 18446744073709551615ull);
        } // 18446744073709551615

        {
            std::stringStream in;
            in << "{ \"test\" : 1.1 }";
            TJsonValue value;
            UNIT_ASSERT(ReadJsonTree(&in, &value));
            UNIT_ASSERT(value.Has("test"));
            UNIT_ASSERT(!value["test"].IsInteger());
            UNIT_ASSERT(!value["test"].IsUInteger());
            UNIT_ASSERT_EQUAL(value["test"].GetInteger(), 0);
            UNIT_ASSERT_EQUAL(value["test"].GetIntegerRobust(), static_cast<long long>(1.1));
            UNIT_ASSERT_EQUAL(value["test"].GetUInteger(), 0);
            UNIT_ASSERT_EQUAL(value["test"].GetUIntegerRobust(), static_cast<unsigned long long>(1.1));
        } // 1.1

        {
            std::stringStream in;
            in << "{ \"test\" : [1, 18446744073709551615] }";
            TJsonValue value;
            UNIT_ASSERT(ReadJsonTree(&in, &value));
            UNIT_ASSERT(value.Has("test"));
            UNIT_ASSERT(value["test"].IsArray());
            UNIT_ASSERT_EQUAL(value["test"].GetArray().size(), 2);
            UNIT_ASSERT(value["test"][0].IsInteger());
            UNIT_ASSERT(value["test"][0].IsUInteger());
            UNIT_ASSERT_EQUAL(value["test"][0].GetInteger(), 1);
            UNIT_ASSERT_EQUAL(value["test"][0].GetUInteger(), 1);
            UNIT_ASSERT(!value["test"][1].IsInteger());
            UNIT_ASSERT(value["test"][1].IsUInteger());
            UNIT_ASSERT_EQUAL(value["test"][1].GetUInteger(), 18446744073709551615ull);
        }
    } // TJsonUnsignedIntegerTest

    Y_UNIT_TEST(TJsonDoubleTest) {
        {
            std::stringStream in;
            in << "{ \"test\" : 1.0 }";
            TJsonValue value;
            UNIT_ASSERT(ReadJsonTree(&in, &value));
            UNIT_ASSERT(value.Has("test"));
            UNIT_ASSERT(value["test"].IsDouble());
            UNIT_ASSERT_EQUAL(value["test"].GetDouble(), 1.0);
            UNIT_ASSERT_EQUAL(value["test"].GetDoubleRobust(), 1.0);
        } // 1.0

        {
            std::stringStream in;
            in << "{ \"test\" : 1 }";
            TJsonValue value;
            UNIT_ASSERT(ReadJsonTree(&in, &value));
            UNIT_ASSERT(value.Has("test"));
            UNIT_ASSERT(value["test"].IsDouble());
            UNIT_ASSERT_EQUAL(value["test"].GetDouble(), 1.0);
            UNIT_ASSERT_EQUAL(value["test"].GetDoubleRobust(), 1.0);
        } // 1

        {
            std::stringStream in;
            in << "{ \"test\" : -1 }";
            TJsonValue value;
            UNIT_ASSERT(ReadJsonTree(&in, &value));
            UNIT_ASSERT(value.Has("test"));
            UNIT_ASSERT(value["test"].IsDouble());
            UNIT_ASSERT_EQUAL(value["test"].GetDouble(), -1.0);
            UNIT_ASSERT_EQUAL(value["test"].GetDoubleRobust(), -1.0);
        } // -1

        {
            std::stringStream in;
            in << "{ \"test\" : " << Max<ui64>() << " }";
            TJsonValue value;
            UNIT_ASSERT(ReadJsonTree(&in, &value));
            UNIT_ASSERT(value.Has("test"));
            UNIT_ASSERT(!value["test"].IsDouble());
            UNIT_ASSERT_EQUAL(value["test"].GetDouble(), 0.0);
            UNIT_ASSERT_EQUAL(value["test"].GetDoubleRobust(), static_cast<double>(Max<ui64>()));
        } // Max<ui64>()
    }     // TJsonDoubleTest

    Y_UNIT_TEST(TJsonInvalidTest) {
        {
            // No exceptions mode.
            std::stringStream in;
            in << "{ \"test\" : }";
            TJsonValue value;
            UNIT_ASSERT(!ReadJsonTree(&in, &value));
        }

        {
            // Exception throwing mode.
            std::stringStream in;
            in << "{ \"test\" : }";
            TJsonValue value;
            UNIT_ASSERT_EXCEPTION(ReadJsonTree(&in, &value, true), TJsonException);
        }
    }

    Y_UNIT_TEST(TJsonMemoryLeakTest) {
        // after https://clubs.at.yandex-team.ru/stackoverflow/3691
        std::string s = ".";
        NJson::TJsonValue json;
        try {
            std::stringInput in(s);
            NJson::ReadJsonTree(&in, &json, true);
        } catch (...) {
        }
    } // TJsonMemoryLeakTest

    Y_UNIT_TEST(TJsonDuplicateKeysWithNullValuesTest) {
        const std::string json = "{\"\":null,\"\":\"\"}";

        std::stringInput in(json);
        NJson::TJsonValue v;
        UNIT_ASSERT(ReadJsonTree(&in, &v));
        UNIT_ASSERT(v.IsMap());
        UNIT_ASSERT_VALUES_EQUAL(1, v.GetMap().size());
        UNIT_ASSERT_VALUES_EQUAL("", v.GetMap().begin()->first);
        UNIT_ASSERT(v.GetMap().begin()->second.IsString());
        UNIT_ASSERT_VALUES_EQUAL("", v.GetMap().begin()->second.GetString());
    }

    // Parsing an extremely deep json tree would result in stack overflow.
    // Not crashing on one is a good indicator of iterative mode.
    Y_UNIT_TEST(TJsonIterativeTest) {
        constexpr ui32 brackets = static_cast<ui32>(1e5);

        std::stringStream jsonStream;
        GenerateDeepJson(jsonStream, brackets);

        TJsonReaderConfig config;
        config.UseIterativeParser = true;
        config.MaxDepth = static_cast<ui32>(1e3);

        TJsonValue v;
        UNIT_ASSERT(!ReadJsonTree(&jsonStream, &config, &v));
    }

    Y_UNIT_TEST(TJsonMaxDepthTest) {
        constexpr ui32 depth = static_cast<ui32>(1e3);

        {
            std::stringStream jsonStream;
            GenerateDeepJson(jsonStream, depth);
            TJsonReaderConfig config;
            config.MaxDepth = depth;
            TJsonValue v;
            UNIT_ASSERT(ReadJsonTree(&jsonStream, &config, &v));
        }

        {
            std::stringStream jsonStream;
            GenerateDeepJson(jsonStream, depth);
            TJsonReaderConfig config;
            config.MaxDepth = depth - 1;
            TJsonValue v;
            UNIT_ASSERT(!ReadJsonTree(&jsonStream, &config, &v));
        }
    }
}


static const std::string YANDEX_STREAMING_JSON("{\"a\":1}//d{\"b\":2}");


Y_UNIT_TEST_SUITE(TCompareReadJsonFast) {
    Y_UNIT_TEST(NoEndl) {
        NJson::TJsonValue parsed;

        bool success = NJson::ReadJsonTree(YANDEX_STREAMING_JSON, &parsed, false);
        bool fast_success = NJson::ReadJsonFastTree(YANDEX_STREAMING_JSON, &parsed, false);
        UNIT_ASSERT(success == fast_success);
    }
    Y_UNIT_TEST(WithEndl) {
        NJson::TJsonValue parsed1;
        NJson::TJsonValue parsed2;

        bool success = NJson::ReadJsonTree(YANDEX_STREAMING_JSON + "\n", &parsed1, false);
        bool fast_success = NJson::ReadJsonFastTree(YANDEX_STREAMING_JSON + "\n", &parsed2, false);

        UNIT_ASSERT_VALUES_EQUAL(success, fast_success);
    }
    Y_UNIT_TEST(NoQuotes) {
        std::string streamingJson = "{a:1}";
        NJson::TJsonValue parsed;

        bool success = NJson::ReadJsonTree(streamingJson, &parsed, false);
        bool fast_success = NJson::ReadJsonFastTree(streamingJson, &parsed, false);
        UNIT_ASSERT(success != fast_success);
    }
}
