#include "connection.h"
#include "descriptor.h"
#include "statement.h"

#include <gtest/gtest.h>
#include <stdexcept>

namespace NYdb::NOdbc {
namespace {

class TOdbcDescriptor : public ::testing::Test {
protected:
    TConnection Connection_;
    TDescriptor Descriptor_{&Connection_};
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

TEST(OdbcStatement, NoOpUnbindPreservesDescriptorGenerations) {
    TConnection connection;
    TStatement statement(&connection);
    SQLINTEGER value = 0;
    ASSERT_EQ(statement.BindCol(1, SQL_C_LONG, &value, sizeof(value), nullptr), SQL_SUCCESS);
    SQLHDESC handle = SQL_NULL_HDESC;
    ASSERT_EQ(statement.GetStmtAttr(SQL_ATTR_APP_ROW_DESC, &handle, 0, nullptr), SQL_SUCCESS);
    auto& descriptor = *CheckedHandle<TDescriptor>(handle);
    TDescriptorSnapshot snapshot;
    ASSERT_TRUE(descriptor.Snapshot(snapshot, true));
    const auto generation = descriptor.GetGeneration();

    ASSERT_EQ(statement.BindCol(2, SQL_C_LONG, nullptr, 0, nullptr), SQL_SUCCESS);
    EXPECT_FALSE(descriptor.Snapshot(snapshot, true));
    EXPECT_EQ(descriptor.GetGeneration(), generation);
    EXPECT_EQ(snapshot.GetRecordCount(), 1);

    SQLINTEGER rebound = 0;
    {
        auto lock = descriptor.LockOperation();
        ASSERT_EQ(descriptor.SetDescField(1, SQL_DESC_DATA_PTR, &rebound, 0), SQL_SUCCESS);
        ASSERT_EQ(descriptor.SetDescField(1, SQL_DESC_NAME, (SQLPOINTER)"renamed", SQL_NTS), SQL_SUCCESS);
    }
    EXPECT_FALSE(descriptor.Snapshot(snapshot, true));
    EXPECT_NE(descriptor.GetGeneration(), generation);
    ASSERT_NE(snapshot.FindRecord(1), nullptr);
    EXPECT_EQ(snapshot.FindRecord(1)->DataPtr, &rebound);
    EXPECT_EQ(snapshot.FindRecord(1)->Name, "renamed");

    {
        auto lock = descriptor.LockOperation();
        ASSERT_EQ(descriptor.SetDescField(1, SQL_DESC_CONCISE_TYPE,
            reinterpret_cast<SQLPOINTER>(SQL_C_SHORT), 0), SQL_SUCCESS);
    }
    EXPECT_TRUE(descriptor.Snapshot(snapshot, true));
    {
        auto lock = descriptor.LockOperation();
        ASSERT_EQ(descriptor.SetDescField(2, SQL_DESC_NAME, (SQLPOINTER)"new", SQL_NTS), SQL_SUCCESS);
    }
    EXPECT_TRUE(descriptor.Snapshot(snapshot, true));
    EXPECT_EQ(snapshot.GetRecordCount(), 2);
    ASSERT_EQ(statement.BindCol(2, SQL_C_LONG, nullptr, 0, nullptr), SQL_SUCCESS);
    EXPECT_TRUE(descriptor.Snapshot(snapshot, true));

    ASSERT_EQ(statement.BindCol(1, SQL_C_LONG, nullptr, 0, nullptr), SQL_SUCCESS);
    EXPECT_TRUE(descriptor.Snapshot(snapshot, true));
    EXPECT_EQ(snapshot.GetRecordCount(), 0);
}

TEST(OdbcStatement, CancelClearsNeedDataAndAllowsReuse) {
    TConnection connection;
    TStatement statement(&connection);
    ASSERT_EQ(statement.Prepare("SELECT ?"), SQL_SUCCESS);
    char token = 0;
    SQLLEN indicator = SQL_DATA_AT_EXEC;
    ASSERT_EQ(statement.BindParameter(1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
        16, 0, &token, 0, &indicator), SQL_SUCCESS);
    ASSERT_EQ(statement.Execute(), SQL_NEED_DATA);
    EXPECT_TRUE(statement.NeedsData());
    char data[] = "cancelled";
    EXPECT_THROW(statement.PutData(data, SQL_NTS), TOdbcException);
    ASSERT_EQ(statement.Cancel(), SQL_SUCCESS);
    EXPECT_FALSE(statement.NeedsData());
    ASSERT_EQ(statement.Execute(), SQL_NEED_DATA);
    SQLPOINTER value = nullptr;
    ASSERT_EQ(statement.ParamData(&value), SQL_NEED_DATA);
    EXPECT_EQ(value, &token);
    ASSERT_EQ(statement.PutData(data, SQL_NTS), SQL_SUCCESS);

    ASSERT_EQ(statement.Cancel(), SQL_SUCCESS);
    EXPECT_FALSE(statement.NeedsData());
    EXPECT_EQ(statement.ParamData(&value), SQL_NO_DATA);
    SQLRETURN result = SQL_ERROR;
    ASSERT_EQ(statement.GetDiagField(0, SQL_DIAG_RETURNCODE, &result, 0, nullptr), SQL_SUCCESS);
    EXPECT_EQ(result, SQL_SUCCESS);

    ASSERT_EQ(statement.Execute(), SQL_NEED_DATA);
    EXPECT_TRUE(statement.NeedsData());
    ASSERT_EQ(statement.ParamData(&value), SQL_NEED_DATA);
    EXPECT_EQ(value, &token);
    EXPECT_EQ(statement.Cancel(), SQL_SUCCESS);

    ASSERT_EQ(statement.Execute(), SQL_NEED_DATA);
    EXPECT_THROW(statement.Prepare("SELECT $p32768"), std::length_error);
    EXPECT_THROW(statement.ParamData(&value), TOdbcException);
    EXPECT_EQ(statement.Cancel(), SQL_SUCCESS);
}

TEST_F(TOdbcDescriptor, ExecuteUsesLatestParameterBinding) {
    TStatement statement(&Connection_);
    ASSERT_EQ(statement.Prepare("SELECT ?"), SQL_SUCCESS);
    char first = 0, second = 0;
    SQLLEN indicator = 0;
    ASSERT_EQ(statement.BindParameter(1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
        16, 0, &first, 0, &indicator), SQL_SUCCESS);
    ASSERT_EQ(statement.BindParameter(1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
        16, 0, &second, 0, &indicator), SQL_SUCCESS);
    indicator = SQL_DATA_AT_EXEC;
    ASSERT_EQ(statement.Execute(), SQL_NEED_DATA);
    SQLPOINTER token = nullptr;
    ASSERT_EQ(statement.ParamData(&token), SQL_NEED_DATA);
    EXPECT_EQ(token, &second);
    char data[] = "value";
    EXPECT_EQ(statement.PutData(data, SQL_NTS), SQL_SUCCESS);
    SQLSMALLINT type = 0;
    ASSERT_EQ(statement.DescribeParam(1, &type, nullptr, nullptr, nullptr), SQL_SUCCESS);
    EXPECT_EQ(type, SQL_VARCHAR);

    SQLHDESC handle = SQL_NULL_HDESC;
    ASSERT_EQ(statement.GetStmtAttr(SQL_ATTR_IMP_PARAM_DESC, &handle, 0, nullptr), SQL_SUCCESS);
    auto& descriptor = *CheckedHandle<TDescriptor>(handle);
    TDescriptorSnapshot snapshot;
    ASSERT_TRUE(descriptor.Snapshot(snapshot));
    {
        auto lock = descriptor.LockOperation();
        ASSERT_EQ(descriptor.SetDescField(1, SQL_DESC_LENGTH,
            reinterpret_cast<SQLPOINTER>(32), 0), SQL_SUCCESS);
    }
    EXPECT_TRUE(descriptor.Snapshot(snapshot));
    {
        auto lock = descriptor.LockOperation();
        ASSERT_EQ(descriptor.SetDescField(1, SQL_DESC_SCALE,
            reinterpret_cast<SQLPOINTER>(3), 0), SQL_SUCCESS);
    }
    EXPECT_TRUE(descriptor.Snapshot(snapshot));
    ASSERT_EQ(statement.BindParameter(1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_TYPE_TIMESTAMP,
        32, 3, &second, 0, &indicator), SQL_SUCCESS);
    EXPECT_TRUE(descriptor.Snapshot(snapshot));
    ASSERT_EQ(statement.DescribeParam(1, &type, nullptr, nullptr, nullptr), SQL_SUCCESS);
    EXPECT_EQ(type, SQL_TYPE_TIMESTAMP);
    EXPECT_TRUE(descriptor.Snapshot(snapshot));
    ASSERT_NE(snapshot.FindRecord(1), nullptr);
    EXPECT_EQ(snapshot.FindRecord(1)->WideTemporalType, EWideTemporal::None);
}

} // namespace
} // namespace NYdb::NOdbc
