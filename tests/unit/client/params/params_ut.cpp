#include <ydb-cpp-sdk/client/params/params.h>

#include <library/cpp/testing/unittest/registar.h>
#include <library/cpp/testing/unittest/tests_data.h>
#include <google/protobuf/text_format.h>
#include <src/api/protos/ydb_value.pb.h>

using namespace NYdb;

using TExpectedErrorException = yexception;

void CheckProtoValue(const Ydb::Value& value, const TString& expected) {
    TStringType protoStr;
    google::protobuf::TextFormat::PrintToString(value, &protoStr);
    UNIT_ASSERT_NO_DIFF(protoStr, expected);
}

Y_UNIT_TEST_SUITE(ParamsBuilder) {
    Y_UNIT_TEST(Build) {
        auto params = TParamsBuilder()
            .AddParam("$param1")
                .BeginList()
                .AddListItem()
                    .BeginOptional()
                        .Uint64(10)
                    .EndOptional()
                .AddListItem()
                    .EmptyOptional()
                .EndList()
                .Build()
            .AddParam("$param2")
                .String("test")
                .Build()
            .Build();

        UNIT_ASSERT_NO_DIFF(FormatType(params.GetValue("$param1")->GetType()), "List<Uint64?>");

        CheckProtoValue(params.GetValue("$param1")->GetProto(),
            "items {\n"
            "  uint64_value: 10\n"
            "}\n"
            "items {\n"
            "  null_flag_value: NULL_VALUE\n"
            "}\n");

        UNIT_ASSERT_NO_DIFF(FormatType(params.GetValue("$param2")->GetType()), "String");
        CheckProtoValue(params.GetValue("$param2")->GetProto(), "bytes_value: \"test\"\n");
    }

    Y_UNIT_TEST(BuildFromValue) {
        auto value2 = TValueBuilder()
            .BeginList()
            .AddListItem()
                .BeginOptional()
                    .BeginTuple()
                    .AddElement()
                        .Int32(-11)
                    .AddElement()
                        .String("test2")
                    .EndTuple()
                .EndOptional()
            .AddListItem()
                .EmptyOptional()
            .EndList()
            .Build();

        auto params = TParamsBuilder()
            .AddParam("$param1")
                .Utf8("test1")
                .Build()
            .AddParam("$param2", value2)
            .Build();

        UNIT_ASSERT_NO_DIFF(FormatType(params.GetValue("$param1")->GetType()), "Utf8");

        CheckProtoValue(params.GetValue("$param1")->GetProto(), "text_value: \"test1\"\n");

        UNIT_ASSERT_NO_DIFF(FormatType(params.GetValue("$param2")->GetType()), "List<Tuple<Int32,String>?>");
        CheckProtoValue(params.GetValue("$param2")->GetProto(),
            "items {\n"
            "  items {\n"
            "    int32_value: -11\n"
            "  }\n"
            "  items {\n"
            "    bytes_value: \"test2\"\n"
            "  }\n"
            "}\n"
            "items {\n"
            "  null_flag_value: NULL_VALUE\n"
            "}\n");
    }

    Y_UNIT_TEST(BuildWithTypeInfo) {
        auto param1Type = TTypeBuilder()
            .BeginList()
                .Primitive(EPrimitiveType::String)
            .EndList()
            .Build();

        auto param2Type = TTypeBuilder()
            .BeginOptional()
                .Primitive(EPrimitiveType::Uint32)
            .EndOptional()
            .Build();

        std::map<std::string, TType> paramsMap;
        paramsMap.emplace("$param1", param1Type);
        paramsMap.emplace("$param2", param2Type);

        auto value1 = TValueBuilder()
            .BeginList()
            .AddListItem()
                .String("str1")
            .AddListItem()
                .String("str2")
            .EndList()
            .Build();

        auto params = TParamsBuilder(paramsMap)
            .AddParam("$param1", value1)
            .AddParam("$param2")
                .EmptyOptional()
                .Build()
            .Build();

        UNIT_ASSERT_NO_DIFF(FormatType(params.GetValue("$param1")->GetType()), "List<String>");
        CheckProtoValue(params.GetValue("$param1")->GetProto(),
            "items {\n"
            "  bytes_value: \"str1\"\n"
            "}\n"
            "items {\n"
            "  bytes_value: \"str2\"\n"
            "}\n");

        UNIT_ASSERT_NO_DIFF(FormatType(params.GetValue("$param2")->GetType()), "Uint32?");
        CheckProtoValue(params.GetValue("$param2")->GetProto(), "null_flag_value: NULL_VALUE\n");
    }

    Y_UNIT_TEST(MissingParam) {
        auto param1Type = TTypeBuilder()
            .BeginList()
                .Primitive(EPrimitiveType::String)
            .EndList()
            .Build();

        auto param2Type = TTypeBuilder()
            .BeginOptional()
                .Primitive(EPrimitiveType::Uint32)
            .EndOptional()
            .Build();

        std::map<std::string, TType> paramsMap;
        paramsMap.emplace("$param1", param1Type);
        paramsMap.emplace("$param2", param2Type);

        try {
            auto params = TParamsBuilder(paramsMap)
                .AddParam("$param3")
                    .EmptyOptional()
                    .Build()
                .Build();
        } catch (const TExpectedErrorException& e) {
            return;
        }

        UNIT_ASSERT(false);
    }

    Y_UNIT_TEST(IncompleteParam) {
        auto paramsBuilder = TParamsBuilder();

        auto& param1Builder = paramsBuilder.AddParam("$param1");
        auto& param2Builder = paramsBuilder.AddParam("$param2");

        param1Builder
            .BeginList()
            .AddListItem()
                .BeginOptional()
                    .Uint64(10)
                .EndOptional()
            .AddListItem()
                .EmptyOptional()
            .EndList()
            .Build();

        param2Builder.String("test");

        try {
            paramsBuilder.Build();
        } catch (const TExpectedErrorException& e) {
            return;
        }

        UNIT_ASSERT(false);
    }

    Y_UNIT_TEST(TypeMismatch) {
        auto param1Type = TTypeBuilder()
            .BeginList()
                .Primitive(EPrimitiveType::String)
            .EndList()
            .Build();

        auto param2Type = TTypeBuilder()
            .BeginOptional()
                .Primitive(EPrimitiveType::Uint32)
            .EndOptional()
            .Build();

        std::map<std::string, TType> paramsMap;
        paramsMap.emplace("$param1", param1Type);
        paramsMap.emplace("$param2", param2Type);

        try {
            auto params = TParamsBuilder(paramsMap)
                .AddParam("$param1")
                    .EmptyOptional()
                    .Build()
                .Build();
        } catch (const TExpectedErrorException& e) {
            return;
        }

        UNIT_ASSERT(false);
    }

    Y_UNIT_TEST(TypeMismatchFromValue) {
        auto param1Type = TTypeBuilder()
            .BeginList()
                .Primitive(EPrimitiveType::String)
            .EndList()
            .Build();

        auto param2Type = TTypeBuilder()
            .BeginOptional()
                .Primitive(EPrimitiveType::Uint32)
            .EndOptional()
            .Build();

        std::map<std::string, TType> paramsMap;
        paramsMap.emplace("$param1", param1Type);
        paramsMap.emplace("$param2", param2Type);

        auto value1 = TValueBuilder()
            .BeginList()
            .AddListItem()
                .String("str1")
            .AddListItem()
                .String("str2")
            .EndList()
            .Build();

        try {
            auto params = TParamsBuilder(paramsMap)
                .AddParam("$param2", value1)
                .Build();
        } catch (const TExpectedErrorException& e) {
            return;
        }

        UNIT_ASSERT(false);
    }
}
