#include "connection.h"
#include "descriptor.h"
#include "statement.h"

#include <gtest/gtest.h>

namespace NYdb::NOdbc {
namespace {

class TOdbcDescriptor : public ::testing::Test {
protected:
    TConnection Connection_;
    TDescriptor Descriptor_{EDescType::Explicit, &Connection_};
    std::unique_lock<std::mutex> Lock_ = Descriptor_.LockOperation();
};

TEST_F(TOdbcDescriptor, ScalarUpdatePreservesOtherFields) {
    std::string name(256, 'x');
    ASSERT_EQ(Descriptor_.SetDescField(1, SQL_DESC_NAME, name.data(), SQL_NTS), SQL_SUCCESS);
    SQLINTEGER value = 42;
    ASSERT_EQ(Descriptor_.SetDescField(1, SQL_DESC_DATA_PTR, &value, 0), SQL_SUCCESS);
    ASSERT_EQ(Descriptor_.SetDescField(1, SQL_DESC_CONCISE_TYPE,
        reinterpret_cast<SQLPOINTER>(SQL_C_LONG), 0), SQL_SUCCESS);

    char output[257] = {};
    ASSERT_EQ(Descriptor_.GetDescField(1, SQL_DESC_NAME, output, sizeof(output), nullptr), SQL_SUCCESS);
    EXPECT_EQ(output, name);
    SQLPOINTER pointer = nullptr;
    ASSERT_EQ(Descriptor_.GetDescField(1, SQL_DESC_DATA_PTR, &pointer, 0, nullptr), SQL_SUCCESS);
    EXPECT_EQ(pointer, &value);
    SQLSMALLINT type = 0;
    ASSERT_EQ(Descriptor_.GetDescField(1, SQL_DESC_CONCISE_TYPE, &type, 0, nullptr), SQL_SUCCESS);
    EXPECT_EQ(type, SQL_C_LONG);
}

TEST_F(TOdbcDescriptor, RejectedFieldsLeaveDescriptorUnchanged) {
    char name[] = "original";
    ASSERT_EQ(Descriptor_.SetDescField(1, SQL_DESC_NAME, name, SQL_NTS), SQL_SUCCESS);
    const auto generation = Descriptor_.GetGeneration();

    EXPECT_EQ(Descriptor_.SetDescField(2, SQL_DESC_NAME, name, -7), SQL_ERROR);
    SQLCHAR state[6] = {};
    ASSERT_EQ(Descriptor_.GetDiagRec(1, state, nullptr, nullptr, 0, nullptr), SQL_SUCCESS);
    EXPECT_STREQ(reinterpret_cast<char*>(state), "HY090");
    EXPECT_EQ(Descriptor_.SetDescField(2, SQL_DESC_NAME, nullptr, SQL_NTS), SQL_ERROR);
    EXPECT_EQ(Descriptor_.SetDescField(2, 32767, nullptr, 0), SQL_ERROR);
    EXPECT_THROW(Descriptor_.SetDescField(0, SQL_DESC_CONCISE_TYPE,
        reinterpret_cast<SQLPOINTER>(SQL_C_LONG), 0), TOdbcException);
    EXPECT_EQ(Descriptor_.GetGeneration(), generation);

    SQLSMALLINT count = 0;
    ASSERT_EQ(Descriptor_.GetDescField(0, SQL_DESC_COUNT, &count, 0, nullptr), SQL_SUCCESS);
    EXPECT_EQ(count, 1);
    char output[16] = {};
    ASSERT_EQ(Descriptor_.GetDescField(1, SQL_DESC_NAME, output, sizeof(output), nullptr), SQL_SUCCESS);
    EXPECT_STREQ(output, name);
}

TEST_F(TOdbcDescriptor, FetchUsesLatestColumnBinding) {
    TStatement statement(&Connection_);
    ASSERT_EQ(statement.GetTypeInfo(SQL_INTEGER), SQL_SUCCESS);
    SQLINTEGER first = 111, second = 222;
    ASSERT_EQ(statement.BindCol(2, SQL_C_LONG, &first, sizeof(first), nullptr), SQL_SUCCESS);
    ASSERT_EQ(statement.BindCol(2, SQL_C_LONG, &second, sizeof(second), nullptr), SQL_SUCCESS);
    ASSERT_EQ(statement.Fetch(), SQL_SUCCESS);
    EXPECT_EQ(first, 111);
    EXPECT_EQ(second, SQL_INTEGER);
}

TEST_F(TOdbcDescriptor, ExecuteUsesLatestParameterBinding) {
    TStatement statement(&Connection_);
    ASSERT_EQ(statement.Prepare("SELECT ?"), SQL_SUCCESS);
    char first = 0, second = 0;
    SQLLEN indicator = SQL_DATA_AT_EXEC;
    ASSERT_EQ(statement.BindParameter(1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
        16, 0, &first, 0, &indicator), SQL_SUCCESS);
    ASSERT_EQ(statement.BindParameter(1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
        16, 0, &second, 0, &indicator), SQL_SUCCESS);
    ASSERT_EQ(statement.Execute(), SQL_NEED_DATA);
    SQLPOINTER token = nullptr;
    ASSERT_EQ(statement.ParamData(&token), SQL_NEED_DATA);
    EXPECT_EQ(token, &second);
    char data[] = "value";
    EXPECT_EQ(statement.PutData(data, SQL_NTS), SQL_SUCCESS);
    SQLSMALLINT type = 0;
    ASSERT_EQ(statement.DescribeParam(1, &type, nullptr, nullptr, nullptr), SQL_SUCCESS);
    EXPECT_EQ(type, SQL_VARCHAR);
}

} // namespace
} // namespace NYdb::NOdbc
