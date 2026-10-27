#include "utils/convert.h"
#undef BOOL

#include <ydb-cpp-sdk/client/params/params.h>

#include <src/api/protos/ydb_value.pb.h>

#include <google/protobuf/text_format.h>

#include <gtest/gtest.h>

#include <tuple>

using namespace NYdb::NOdbc;
using namespace NYdb;

template<typename T>
void CheckProto(const T& value, const std::string& expected) {
    std::string protoStr;
    google::protobuf::TextFormat::PrintToString(value, &protoStr);
    ASSERT_EQ(protoStr, expected);
}

TEST(OdbcConvert, Int64ToYdb) {
    SQLBIGINT v = 42;
    TBoundParam param{
        1, // ParamNumber
        SQL_C_SBIGINT, // ValueType
        SQL_BIGINT, // ParameterType
        0, 0, // ColumnSize, DecimalDigits
        &v, // ParameterValuePtr
        sizeof(v), // BufferLength
        nullptr // StrLenOrIndPtr
    };

    TParamsBuilder paramsBuilder;
    ConvertParam(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProto(value->GetType().GetProto(), "type_id: INT64\n");
    CheckProto(value->GetProto(), "int64_value: 42\n");
}

TEST(OdbcConvert, UnsignedCSelectsYdbUnsignedType) {
    SQLUBIGINT v = 123;
    TBoundParam param{
        1, SQL_C_UBIGINT, SQL_BIGINT, 0, 0, &v, sizeof(v), nullptr
    };
    TParamsBuilder paramsBuilder;
    ConvertParam(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProto(value->GetType().GetProto(), "type_id: UINT64\n");
    CheckProto(value->GetProto(), "uint64_value: 123\n");
}

TEST(OdbcConvert, WideStringToYdbUtf8) {
    SQLWCHAR text[] = {'h', 'e', 'l', 'l', 'o', 0};
    SQLLEN length = SQL_NTS;
    TBoundParam param{
        1, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0,
        text, sizeof(text), &length
    };
    TParamsBuilder paramsBuilder;
    ASSERT_EQ(ConvertParam(param, paramsBuilder.AddParam("$p1")), SQL_SUCCESS);
    const auto value = paramsBuilder.Build().GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProto(value->GetType().GetProto(), "type_id: UTF8\n");
    CheckProto(value->GetProto(), "text_value: \"hello\"\n");
}

TEST(OdbcConvert, TimestampStructToYdbTimestamp) {
    SQL_TIMESTAMP_STRUCT timestamp{2024, 6, 15, 14, 30, 20, 123456000};
    TBoundParam param{
        1, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, 0, 0,
        &timestamp, sizeof(timestamp), nullptr
    };
    TParamsBuilder paramsBuilder;
    ASSERT_EQ(ConvertParam(param, paramsBuilder.AddParam("$p1")), SQL_SUCCESS);
    const auto value = paramsBuilder.Build().GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProto(value->GetType().GetProto(), "type_id: TIMESTAMP\n");

    const std::tuple<const char*, SQLSMALLINT, const char*> temporal[] = {
        {"2024-06-15", SQL_TYPE_DATE, ""}, {"14:30:20", SQL_TYPE_TIME, ""},
        {"2024-06-15", SQL_TYPE_TIME, "22018"},
        {" 2024-06-15 14:30:20.123456 ", SQL_TYPE_TIMESTAMP, ""},
        {"2024-06-15 14:30:20.123456789", SQL_TYPE_TIMESTAMP, "22008"},
        {"14:30:20.5", SQL_TYPE_TIMESTAMP, ""},
        {"bad", SQL_TYPE_DATE, "22018"},
    };
    for (const auto& [text, type, state] : temporal) {
        SQLLEN length = SQL_NTS;
        TParamsBuilder builder;
        const TBoundParam parameter{1, SQL_C_CHAR, type, 0, 0, const_cast<char*>(text), 0, &length};
        ASSERT_EQ(ConvertParam(parameter, builder.AddParam("$p1")), *state ? SQL_ERROR : SQL_SUCCESS);
        if (*state) {
            EXPECT_STREQ(ConsumeLastConvertSqlState(), state);
            continue;
        }
        const auto converted = builder.Build().GetValue("$p1");
        TValueParser parser(*converted);
        if (type == SQL_TYPE_DATE) EXPECT_EQ(parser.GetDate(), TInstant::Days(19889));
        else if (type == SQL_TYPE_TIME) EXPECT_EQ(parser.GetDatetime(), TInstant::Seconds(52220));
        else if (text[2] == ':') {
            EXPECT_EQ(parser.GetTimestamp().MicroSeconds() % TDuration::Days(1).MicroSeconds(), 52220500000ULL);
        } else EXPECT_EQ(parser.GetTimestamp(), TInstant::MicroSeconds(1718461820123456));
    }
}

TEST(OdbcConvert, DoubleToYdb) {
    SQLDOUBLE v = 3.14;
    TBoundParam param{
        1, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &v, sizeof(v), nullptr
    };
    TParamsBuilder paramsBuilder;
    ConvertParam(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProto(value->GetType().GetProto(), "type_id: DOUBLE\n");
    CheckProto(value->GetProto(), "double_value: 3.14\n");
}

TEST(OdbcConvert, DoubleToYdbDecimalPreservesPrecisionAndScale) {
    SQLDOUBLE v = 123.456;
    TBoundParam param{
        1, SQL_C_DOUBLE, SQL_DECIMAL, 18, 5, &v, sizeof(v), nullptr
    };
    TParamsBuilder paramsBuilder;
    ASSERT_EQ(ConvertParam(param, paramsBuilder.AddParam("$p1")), SQL_SUCCESS);
    const auto value = paramsBuilder.Build().GetValue("$p1");
    ASSERT_TRUE(value);

    TValueParser parser(*value);
    const auto decimal = parser.GetDecimal();
    EXPECT_EQ(decimal.DecimalType_.Precision, 18);
    EXPECT_EQ(decimal.DecimalType_.Scale, 5);
    EXPECT_EQ(decimal.ToString(), "123.456");

    char text[16] = {};
    SQLLEN textLength = 0;
    ASSERT_EQ(ConvertColumn(parser, SQL_C_CHAR, text, sizeof(text), &textLength), SQL_SUCCESS);
    EXPECT_EQ(textLength, 9);
    EXPECT_STREQ(text, "123.45600");
}

TEST(OdbcConvert, StringToYdbUtf8) {
    const char* str = "hello";
    SQLLEN len = 5;
    TBoundParam param{
        1, SQL_C_CHAR, SQL_VARCHAR, 0, 0, (SQLPOINTER)str, len, nullptr
    };
    TParamsBuilder paramsBuilder;
    ConvertParam(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProto(value->GetType().GetProto(), "type_id: UTF8\n");
    CheckProto(value->GetProto(), "text_value: \"hello\"\n");
}

TEST(OdbcConvert, StringToYdbBinary) {
    const char* str = "bin\x01\x02";
    SQLLEN len = 5;
    TBoundParam param{
        1, SQL_C_BINARY, SQL_BINARY, 0, 0, (SQLPOINTER)str, len, nullptr
    };
    TParamsBuilder paramsBuilder;
    ConvertParam(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProto(value->GetType().GetProto(), "type_id: STRING\n");
    CheckProto(value->GetProto(), "bytes_value: \"bin\\001\\002\"\n");
}

TEST(OdbcConvert, NativeValuesToCharacterParameters) {
    SQLINTEGER integer = 42;
    SQLDOUBLE real = 0.1;
    SQLREAL single = 0.1f;
    SQL_DATE_STRUCT date{1960, 2, 29};
    SQL_TIMESTAMP_STRUCT timestamp{2026, 10, 26, 12, 34, 56, 123456789};
    SQL_TIME_STRUCT time{12, 34, 56};
    const std::pair<TBoundParam, const char*> cases[] = {
        {{1, SQL_C_LONG, SQL_VARCHAR, 0, 0, &integer, 0, nullptr}, "42"},
        {{1, SQL_C_DOUBLE, SQL_VARCHAR, 0, 0, &real, 0, nullptr}, "0.1"},
        {{1, SQL_C_FLOAT, SQL_VARCHAR, 0, 0, &single, 0, nullptr}, "0.1"},
        {{1, SQL_C_TYPE_DATE, SQL_VARCHAR, 0, 0, &date, 0, nullptr}, "1960-02-29"},
        {{1, SQL_C_TYPE_TIMESTAMP, SQL_VARCHAR, 0, 0, &timestamp, 0, nullptr}, "2026-10-26 12:34:56.123456789"},
        {{1, SQL_C_TYPE_TIME, SQL_VARCHAR, 0, 0, &time, 0, nullptr}, "12:34:56"},
    };
    for (const auto& [parameter, expected] : cases) {
        TParamsBuilder builder;
        ASSERT_EQ(ConvertParam(parameter, builder.AddParam("$p1")), SQL_SUCCESS);
        const auto value = builder.Build().GetValue("$p1");
        TValueParser parser(*value);
        EXPECT_EQ(parser.GetUtf8(), expected);
    }
    date.month = 257;
    TParamsBuilder invalid;
    EXPECT_EQ(ConvertParam({1, SQL_C_TYPE_DATE, SQL_VARCHAR, 0, 0, &date, 0, nullptr},
        invalid.AddParam("$p1")), SQL_ERROR);
    EXPECT_STREQ(ConsumeLastConvertSqlState(), "22008");
}

TEST(OdbcConvert, CharacterToNumericParameters) {
    const std::tuple<const char*, SQLSMALLINT, const char*, int64_t> cases[] = {
        {" +42 ", SQL_INTEGER, "", 42}, {"2147483648", SQL_INTEGER, "22001", 0}, {"bad", SQL_INTEGER, "22018", 0},
        {"+-42", SQL_INTEGER, "22018", 0}, {"1.0", SQL_INTEGER, "", 1}, {"1e2", SQL_INTEGER, "", 100},
        {"1.5", SQL_INTEGER, "22001", 0}, {"1e+-2", SQL_INTEGER, "22018", 0},
        {"9007199254740993.0", SQL_BIGINT, "", 9007199254740993LL},
        {"9.223372036854775807e18", SQL_BIGINT, "", 9223372036854775807LL},
        {"0e999999999999", SQL_INTEGER, "", 0}, {"1.00000000000000000001", SQL_INTEGER, "22001", 0},
        {"1.00000000000000000001", SQL_BIT, "22001", 0},
        {"1.25", SQL_DOUBLE, "", 0}, {"1e999", SQL_DOUBLE, "22003", 0},
        {"0x10", SQL_DOUBLE, "22018", 0}, {"0X2", SQL_BIT, "22018", 0},
    };
    for (const auto& [text, type, state, expected] : cases) {
        SQLLEN length = SQL_NTS;
        const TBoundParam parameter{1, SQL_C_CHAR, type, 10, 0,
            const_cast<char*>(text), 0, &length};
        TParamsBuilder builder;
        EXPECT_EQ(ConvertParam(parameter, builder.AddParam("$p1")), *state ? SQL_ERROR : SQL_SUCCESS);
        if (*state) {
            EXPECT_STREQ(ConsumeLastConvertSqlState(), state);
        } else {
            const auto value = builder.Build().GetValue("$p1");
            TValueParser parser(*value);
            if (type == SQL_INTEGER) EXPECT_EQ(parser.GetInt32(), expected);
            else if (type == SQL_BIGINT) EXPECT_EQ(parser.GetInt64(), expected);
            else EXPECT_EQ(parser.GetDouble(), 1.25);
        }
    }
    SQLINTEGER integer = 42;
    SQLDOUBLE real = 42.75, overflow = 9223372036854775808.0;
    const std::pair<TBoundParam, const char*> numeric[] = {
        {{1, SQL_C_LONG, SQL_DOUBLE, 0, 0, &integer, 0, nullptr}, ""},
        {{1, SQL_C_DOUBLE, SQL_INTEGER, 0, 0, &real, 0, nullptr}, ""},
        {{1, SQL_C_DOUBLE, SQL_BIGINT, 0, 0, &overflow, 0, nullptr}, "22003"},
    };
    for (const auto& [parameter, state] : numeric) {
        TParamsBuilder builder;
        EXPECT_EQ(ConvertParam(parameter, builder.AddParam("$p1")), *state ? SQL_ERROR : SQL_SUCCESS);
        if (*state) EXPECT_STREQ(ConsumeLastConvertSqlState(), state);
        else {
            const auto value = builder.Build().GetValue("$p1");
            TValueParser parser(*value);
            if (parameter.ParameterType == SQL_DOUBLE) EXPECT_EQ(parser.GetDouble(), 42);
            else EXPECT_EQ(parser.GetInt32(), 42);
        }
    }
}

TEST(OdbcConvert, Int64NullToYdb) {
    SQLBIGINT v = 42;
    SQLLEN nullInd = SQL_NULL_DATA;
    TBoundParam param{
        1, SQL_C_SBIGINT, SQL_BIGINT, 0, 0, &v, sizeof(v), &nullInd
    };
    TParamsBuilder paramsBuilder;
    ConvertParam(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProto(value->GetType().GetProto(), "optional_type {\n  item {\n    type_id: INT64\n  }\n}\n");
    CheckProto(value->GetProto(), "null_flag_value: NULL_VALUE\n");
}

TEST(OdbcConvert, StringNullToYdb) {
    const char* str = "test";
    SQLLEN nullInd = SQL_NULL_DATA;
    TBoundParam param{
        1, SQL_C_CHAR, SQL_VARCHAR, 0, 0, (SQLPOINTER)str, 4, &nullInd
    };
    TParamsBuilder paramsBuilder;
    ConvertParam(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProto(value->GetType().GetProto(), "optional_type {\n  item {\n    type_id: UTF8\n  }\n}\n");
    CheckProto(value->GetProto(), "null_flag_value: NULL_VALUE\n");
}

TEST(OdbcConvert, Int32ToYdb) {
    SQLINTEGER v = 42;
    TBoundParam param{
        1, SQL_C_LONG, SQL_INTEGER, 0, 0, &v, sizeof(v), nullptr
    };
    TParamsBuilder paramsBuilder;
    ConvertParam(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProto(value->GetType().GetProto(), "type_id: INT32\n");
    CheckProto(value->GetProto(), "int32_value: 42\n");
}

TEST(OdbcConvert, NonNullValueCanUseOptionalDeclaredType) {
    SQLINTEGER v = 42;
    TBoundParam param{
        1, SQL_C_LONG, SQL_INTEGER, 0, 0, &v, sizeof(v), nullptr
    };
    TParamsBuilder paramsBuilder;
    ASSERT_EQ(ConvertParam(param, paramsBuilder.AddParam("$p1"), true), SQL_SUCCESS);
    const auto value = paramsBuilder.Build().GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProto(value->GetType().GetProto(),
        "optional_type {\n  item {\n    type_id: INT32\n  }\n}\n");
    CheckProto(value->GetProto(), "int32_value: 42\n");
}

TEST(OdbcConvert, Int32NegativeToYdb) {
    SQLINTEGER v = -999;
    TBoundParam param{
        1, SQL_C_LONG, SQL_INTEGER, 0, 0, &v, sizeof(v), nullptr
    };
    TParamsBuilder paramsBuilder;
    ConvertParam(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProto(value->GetProto(), "int32_value: -999\n");
}

TEST(OdbcConvert, Int32ZeroToYdb) {
    SQLINTEGER v = 0;
    TBoundParam param{
        1, SQL_C_LONG, SQL_INTEGER, 0, 0, &v, sizeof(v), nullptr
    };
    TParamsBuilder paramsBuilder;
    ConvertParam(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProto(value->GetProto(), "int32_value: 0\n");
}

TEST(OdbcConvert, Int32MaxToYdb) {
    SQLINTEGER v = 2147483647;  // INT32_MAX
    TBoundParam param{
        1, SQL_C_LONG, SQL_INTEGER, 0, 0, &v, sizeof(v), nullptr
    };
    TParamsBuilder paramsBuilder;
    ConvertParam(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProto(value->GetProto(), "int32_value: 2147483647\n");
}

TEST(OdbcConvert, Int32NullToYdb) {
    SQLINTEGER v = 42;
    SQLLEN nullInd = SQL_NULL_DATA;
    TBoundParam param{
        1, SQL_C_LONG, SQL_INTEGER, 0, 0, &v, sizeof(v), &nullInd
    };
    TParamsBuilder paramsBuilder;
    ConvertParam(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProto(value->GetProto(), "null_flag_value: NULL_VALUE\n");
}

TEST(OdbcConvert, Int64NegativeToYdb) {
    SQLBIGINT v = -123456789012345LL;
    TBoundParam param{
        1, SQL_C_SBIGINT, SQL_BIGINT, 0, 0, &v, sizeof(v), nullptr
    };
    TParamsBuilder paramsBuilder;
    ConvertParam(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProto(value->GetProto(), "int64_value: -123456789012345\n");
}

TEST(OdbcConvert, Int64ZeroToYdb) {
    SQLBIGINT v = 0;
    TBoundParam param{
        1, SQL_C_SBIGINT, SQL_BIGINT, 0, 0, &v, sizeof(v), nullptr
    };
    TParamsBuilder paramsBuilder;
    ConvertParam(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProto(value->GetProto(), "int64_value: 0\n");
}

TEST(OdbcConvert, DoubleNegativeToYdb) {
    SQLDOUBLE v = -2.71828;
    TBoundParam param{
        1, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &v, sizeof(v), nullptr
    };
    TParamsBuilder paramsBuilder;
    ConvertParam(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
}

TEST(OdbcConvert, DoubleZeroToYdb) {
    SQLDOUBLE v = 0.0;
    TBoundParam param{
        1, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &v, sizeof(v), nullptr
    };
    TParamsBuilder paramsBuilder;
    ConvertParam(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProto(value->GetProto(), "double_value: 0\n");
}

TEST(OdbcConvert, DoubleNullToYdb) {
    SQLDOUBLE v = 3.14;
    SQLLEN nullInd = SQL_NULL_DATA;
    TBoundParam param{
        1, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &v, sizeof(v), &nullInd
    };
    TParamsBuilder paramsBuilder;
    ConvertParam(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProto(value->GetProto(), "null_flag_value: NULL_VALUE\n");
}

TEST(OdbcConvert, StringEmptyToYdb) {
    const char* str = "";
    SQLLEN len = 0;
    TBoundParam param{
        1, SQL_C_CHAR, SQL_VARCHAR, 0, 0, (SQLPOINTER)str, len, &len
    };
    TParamsBuilder paramsBuilder;
    ConvertParam(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProto(value->GetProto(), "text_value: \"\"\n");
}

TEST(OdbcConvert, StringUnicodeToYdb) {
    const char* str = "Привет";
    SQLLEN len = SQL_NTS;
    TBoundParam param{
        1, SQL_C_CHAR, SQL_VARCHAR, 0, 0, (SQLPOINTER)str, 0, &len
    };
    TParamsBuilder paramsBuilder;
    ConvertParam(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
}

TEST(OdbcConvert, StringWithLengthToYdb) {
    const char* str = "hello world";
    SQLLEN len = 5;  // Only "hello"
    TBoundParam param{
        1, SQL_C_CHAR, SQL_VARCHAR, 0, 0, (SQLPOINTER)str, len, &len
    };
    TParamsBuilder paramsBuilder;
    ConvertParam(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProto(value->GetProto(), "text_value: \"hello\"\n");
}

TEST(OdbcConvert, StringNullTerminatedToYdb) {
    const char* str = "test";
    SQLLEN len = SQL_NTS;
    TBoundParam param{
        1, SQL_C_CHAR, SQL_VARCHAR, 0, 0, (SQLPOINTER)str, 0, &len
    };
    TParamsBuilder paramsBuilder;
    ConvertParam(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProto(value->GetProto(), "text_value: \"test\"\n");
}


TEST(OdbcConvert, BinaryNullToYdb) {
    const char* data = "\x01\x02\x03";
    SQLLEN nullInd = SQL_NULL_DATA;
    TBoundParam param{
        1, SQL_C_BINARY, SQL_BINARY, 0, 0, (SQLPOINTER)data, 3, &nullInd
    };
    TParamsBuilder paramsBuilder;
    ConvertParam(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProto(value->GetProto(), "null_flag_value: NULL_VALUE\n");
}

TEST(OdbcConvert, BinaryEmptyToYdb) {
    const char* data = "";
    SQLLEN len = 0;
    TBoundParam param{
        1, SQL_C_BINARY, SQL_BINARY, 0, 0, (SQLPOINTER)data, len, &len
    };
    TParamsBuilder paramsBuilder;
    ConvertParam(param, paramsBuilder.AddParam("$p1"));
    auto params = paramsBuilder.Build();
    auto value = params.GetValue("$p1");
    ASSERT_TRUE(value);
    CheckProto(value->GetProto(), "bytes_value: \"\"\n");
}

TEST(OdbcConvert, VirtualScalarUsesSharedIntegerConversion) {
    const TOdbcScalar value = int64_t{123};
    SQLSMALLINT result = 0;
    SQLLEN length = 0;

    ASSERT_EQ(ConvertColumn(value, SQL_C_SHORT, &result, sizeof(result), &length), SQL_SUCCESS);
    EXPECT_EQ(result, 123);
    EXPECT_EQ(length, sizeof(result));
}

TEST(OdbcConvert, VirtualScalarPreservesRangeSqlState) {
    const TOdbcScalar value = uint64_t{256};
    SQLCHAR result = 0;

    EXPECT_EQ(ConvertColumn(value, SQL_C_UTINYINT, &result, sizeof(result), nullptr), SQL_ERROR);
    EXPECT_STREQ(ConsumeLastConvertSqlState(), "22003");
}

TEST(OdbcConvert, VirtualScalarNullRequiresIndicator) {
    const TOdbcScalar value = std::monostate{};

    EXPECT_EQ(ConvertColumn(value, SQL_C_CHAR, nullptr, 0, nullptr), SQL_ERROR);
    EXPECT_STREQ(ConsumeLastConvertSqlState(), "22002");
}

TEST(OdbcConvert, InvalidUtf8ToWidePreservesConversionState) {
    const TOdbcScalar value = std::string("\xff", 1);
    SQLWCHAR output[2] = {};

    EXPECT_EQ(ConvertColumn(value, SQL_C_WCHAR, output, sizeof(output), nullptr), SQL_ERROR);
    EXPECT_STREQ(ConsumeLastConvertSqlState(), "22018");
}
