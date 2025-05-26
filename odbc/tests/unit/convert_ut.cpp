#include <odbc/src/utils/convert.h>
#undef BOOL

#include <ydb-cpp-sdk/client/params/params.h>

#include <src/api/protos/ydb_value.pb.h>

#include <google/protobuf/text_format.h>

#include <gtest/gtest.h>

using namespace NYdb::NOdbc;
using namespace NYdb;

void CheckProtoValue(const Ydb::Value& value, const std::string& expected) {
    std::string protoStr;
    google::protobuf::TextFormat::PrintToString(value, &protoStr);
    ASSERT_EQ(protoStr, expected);
}

TEST(OdbcConvert, Int64ToYdb) {
    SQLBIGINT v = 42;
    TBoundParam param{
        1, // ParamNumber
        SQL_PARAM_INPUT, // InputOutputType
        SQL_C_SBIGINT, // ValueType
        SQL_BIGINT, // ParameterType
        0, 0, // ColumnSize, DecimalDigits
        &v, // ParameterValuePtr
        sizeof(v), // BufferLength
        nullptr // StrLenOrIndPtr
    };

    TParamsBuilder paramsBuilder;
    ConvertValue(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProtoValue(value->GetProto(), "int64_value: 42\n");
}

TEST(OdbcConvert, Uint64ToYdb) {
    SQLUBIGINT v = 123;
    TBoundParam param{
        1, SQL_PARAM_INPUT, SQL_C_UBIGINT, SQL_BIGINT, 0, 0, &v, sizeof(v), nullptr
    };
    TParamsBuilder paramsBuilder;
    ConvertValue(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProtoValue(value->GetProto(), "uint64_value: 123\n");
}

TEST(OdbcConvert, DoubleToYdb) {
    SQLDOUBLE v = 3.14;
    TBoundParam param{
        1, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &v, sizeof(v), nullptr
    };
    TParamsBuilder paramsBuilder;
    ConvertValue(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProtoValue(value->GetProto(), "double_value: 3.14\n");
}

TEST(OdbcConvert, StringToYdbUtf8) {
    const char* str = "hello";
    SQLLEN len = 5;
    TBoundParam param{
        1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, (SQLPOINTER)str, len, nullptr
    };
    TParamsBuilder paramsBuilder;
    ConvertValue(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProtoValue(value->GetProto(), "text_value: \"hello\"\n");
}

TEST(OdbcConvert, StringToYdbBinary) {
    const char* str = "bin\x01\x02";
    SQLLEN len = 5;
    TBoundParam param{
        1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_BINARY, 0, 0, (SQLPOINTER)str, len, nullptr
    };
    TParamsBuilder paramsBuilder;
    ConvertValue(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    ASSERT_EQ(value->GetProto().bytes_value(), std::string(str, len));
}

TEST(OdbcConvert, Int64NullToYdb) {
    SQLBIGINT v = 42;
    SQLLEN nullInd = SQL_NULL_DATA;
    TBoundParam param{
        1, SQL_PARAM_INPUT, SQL_C_SBIGINT, SQL_BIGINT, 0, 0, &v, sizeof(v), &nullInd
    };
    TParamsBuilder paramsBuilder;
    ConvertValue(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    ASSERT_EQ(value->GetProto().null_flag_value(), ::google::protobuf::NullValue::NULL_VALUE);
}

TEST(OdbcConvert, StringNullToYdb) {
    const char* str = "test";
    SQLLEN nullInd = SQL_NULL_DATA;
    TBoundParam param{
        1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, (SQLPOINTER)str, 4, &nullInd
    };
    TParamsBuilder paramsBuilder;
    ConvertValue(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    ASSERT_EQ(value->GetProto().null_flag_value(), ::google::protobuf::NullValue::NULL_VALUE);
}
