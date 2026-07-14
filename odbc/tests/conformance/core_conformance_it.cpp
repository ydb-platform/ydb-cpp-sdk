#include "../integration/test_utils.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <tuple>
#include <utility>

namespace {

bool IsOdbcSuccess(SQLRETURN rc) {
    return rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO;
}

::testing::AssertionResult OdbcSuccess(
    SQLRETURN rc,
    SQLHANDLE handle,
    SQLSMALLINT handleType) {
    if (IsOdbcSuccess(rc)) {
        return ::testing::AssertionSuccess();
    }
    return ::testing::AssertionFailure()
        << "ODBC return code " << rc << ": " << GetOdbcError(handle, handleType);
}

struct TCoreFunction {
    SQLUSMALLINT Id;
    const char* Name;
};

// ODBC 3.x Core functions from the Microsoft/Open Group conformance table.
// SQLDataSources and SQLDrivers are Driver Manager functions and are exercised
// separately; all entries below must be advertised by the connected driver.
constexpr std::array<TCoreFunction, 47> CoreDriverFunctions{{
    {SQL_API_SQLALLOCHANDLE, "SQLAllocHandle"},
    {SQL_API_SQLBINDCOL, "SQLBindCol"},
    {SQL_API_SQLBINDPARAMETER, "SQLBindParameter"},
    {SQL_API_SQLCANCEL, "SQLCancel"},
    {SQL_API_SQLCLOSECURSOR, "SQLCloseCursor"},
    {SQL_API_SQLCOLATTRIBUTE, "SQLColAttribute"},
    {SQL_API_SQLCOLUMNS, "SQLColumns"},
    {SQL_API_SQLCONNECT, "SQLConnect"},
    {SQL_API_SQLCOPYDESC, "SQLCopyDesc"},
    {SQL_API_SQLDESCRIBECOL, "SQLDescribeCol"},
    {SQL_API_SQLDISCONNECT, "SQLDisconnect"},
    {SQL_API_SQLDRIVERCONNECT, "SQLDriverConnect"},
    {SQL_API_SQLENDTRAN, "SQLEndTran"},
    {SQL_API_SQLEXECDIRECT, "SQLExecDirect"},
    {SQL_API_SQLEXECUTE, "SQLExecute"},
    {SQL_API_SQLFETCH, "SQLFetch"},
    {SQL_API_SQLFETCHSCROLL, "SQLFetchScroll"},
    {SQL_API_SQLFREEHANDLE, "SQLFreeHandle"},
    {SQL_API_SQLFREESTMT, "SQLFreeStmt"},
    {SQL_API_SQLGETCONNECTATTR, "SQLGetConnectAttr"},
    {SQL_API_SQLGETCURSORNAME, "SQLGetCursorName"},
    {SQL_API_SQLGETDATA, "SQLGetData"},
    {SQL_API_SQLGETDESCFIELD, "SQLGetDescField"},
    {SQL_API_SQLGETDESCREC, "SQLGetDescRec"},
    {SQL_API_SQLGETDIAGFIELD, "SQLGetDiagField"},
    {SQL_API_SQLGETDIAGREC, "SQLGetDiagRec"},
    {SQL_API_SQLGETENVATTR, "SQLGetEnvAttr"},
    {SQL_API_SQLGETFUNCTIONS, "SQLGetFunctions"},
    {SQL_API_SQLGETINFO, "SQLGetInfo"},
    {SQL_API_SQLGETSTMTATTR, "SQLGetStmtAttr"},
    {SQL_API_SQLGETTYPEINFO, "SQLGetTypeInfo"},
    {SQL_API_SQLNATIVESQL, "SQLNativeSql"},
    {SQL_API_SQLNUMPARAMS, "SQLNumParams"},
    {SQL_API_SQLNUMRESULTCOLS, "SQLNumResultCols"},
    {SQL_API_SQLPARAMDATA, "SQLParamData"},
    {SQL_API_SQLPREPARE, "SQLPrepare"},
    {SQL_API_SQLPUTDATA, "SQLPutData"},
    {SQL_API_SQLROWCOUNT, "SQLRowCount"},
    {SQL_API_SQLSETCONNECTATTR, "SQLSetConnectAttr"},
    {SQL_API_SQLSETCURSORNAME, "SQLSetCursorName"},
    {SQL_API_SQLSETDESCFIELD, "SQLSetDescField"},
    {SQL_API_SQLSETDESCREC, "SQLSetDescRec"},
    {SQL_API_SQLSETENVATTR, "SQLSetEnvAttr"},
    {SQL_API_SQLSETSTMTATTR, "SQLSetStmtAttr"},
    {SQL_API_SQLSPECIALCOLUMNS, "SQLSpecialColumns"},
    {SQL_API_SQLSTATISTICS, "SQLStatistics"},
    {SQL_API_SQLTABLES, "SQLTables"},
}};

void ExpectDescriptorField(
    SQLHDESC desc,
    SQLSMALLINT record,
    SQLSMALLINT field,
    const char* name,
    bool characterField = false) {
    alignas(std::max_align_t) std::array<unsigned char, 256> storage{};
    SQLINTEGER length = 0;
    const SQLRETURN rc = SQLGetDescField(
        desc,
        record,
        field,
        storage.data(),
        characterField ? static_cast<SQLINTEGER>(storage.size()) : 0,
        &length);
    EXPECT_TRUE(OdbcSuccess(rc, desc, SQL_HANDLE_DESC)) << "field " << name;
}

class OdbcCoreConformance : public ::testing::Test {
protected:
    void SetUp() override {
        AllocEnvAndConnect(&Env_, &Dbc_);
    }

    void TearDown() override {
        if (Dbc_ != SQL_NULL_HDBC) {
            SQLDisconnect(Dbc_);
            SQLFreeHandle(SQL_HANDLE_DBC, Dbc_);
        }
        if (Env_ != SQL_NULL_HENV) {
            SQLFreeHandle(SQL_HANDLE_ENV, Env_);
        }
    }

    SQLHSTMT AllocStatement() {
        SQLHSTMT stmt = SQL_NULL_HSTMT;
        EXPECT_EQ(SQLAllocHandle(SQL_HANDLE_STMT, Dbc_, &stmt), SQL_SUCCESS);
        return stmt;
    }

    SQLHENV Env_ = SQL_NULL_HENV;
    SQLHDBC Dbc_ = SQL_NULL_HDBC;
};

} // namespace

TEST_F(OdbcCoreConformance, DeclaresAtLeastCoreInterfaceConformance) {
    // SQL_ATTR_ODBC_VERSION is the only Core-level environment attribute.
    SQLINTEGER version = 0;
    ASSERT_TRUE(OdbcSuccess(
        SQLGetEnvAttr(Env_, SQL_ATTR_ODBC_VERSION, &version, sizeof(version), nullptr),
        Env_,
        SQL_HANDLE_ENV));
    EXPECT_EQ(version, SQL_OV_ODBC3);

    SQLUINTEGER level = 0;
    SQLSMALLINT length = 0;
    const SQLRETURN rc = SQLGetInfo(
        Dbc_, SQL_ODBC_INTERFACE_CONFORMANCE, &level, sizeof(level), &length);
    ASSERT_TRUE(OdbcSuccess(rc, Dbc_, SQL_HANDLE_DBC));
    EXPECT_GE(level, static_cast<SQLUINTEGER>(SQL_OIC_CORE));
    EXPECT_EQ(length, sizeof(level));
}

TEST_F(OdbcCoreConformance, AdvertisesEveryMandatoryDriverFunction) {
    std::array<SQLUSMALLINT, SQL_API_ODBC3_ALL_FUNCTIONS_SIZE> bitmap{};
    ASSERT_TRUE(OdbcSuccess(
        SQLGetFunctions(Dbc_, SQL_API_ODBC3_ALL_FUNCTIONS, bitmap.data()),
        Dbc_,
        SQL_HANDLE_DBC));

    for (const auto& function : CoreDriverFunctions) {
        SQLUSMALLINT supported = SQL_FALSE;
        const SQLRETURN rc = SQLGetFunctions(Dbc_, function.Id, &supported);
        EXPECT_TRUE(OdbcSuccess(rc, Dbc_, SQL_HANDLE_DBC)) << function.Name;
        EXPECT_EQ(supported, SQL_TRUE) << function.Name;
        EXPECT_EQ(SQL_FUNC_EXISTS(bitmap.data(), function.Id), SQL_TRUE)
            << function.Name << " missing from SQL_API_ODBC3_ALL_FUNCTIONS";
    }
}

TEST_F(OdbcCoreConformance, DriverManagerEnumerationFunctionsAreUsable) {
    SQLCHAR description[256] = {};
    SQLCHAR attributes[512] = {};
    SQLSMALLINT descriptionLength = 0;
    SQLSMALLINT attributesLength = 0;
    SQLRETURN rc = SQLDrivers(
        Env_,
        SQL_FETCH_FIRST,
        description,
        sizeof(description),
        &descriptionLength,
        attributes,
        sizeof(attributes),
        &attributesLength);
    EXPECT_TRUE(IsOdbcSuccess(rc) || rc == SQL_NO_DATA) << "SQLDrivers returned " << rc;

    SQLCHAR dsn[256] = {};
    SQLCHAR dsnDescription[256] = {};
    SQLSMALLINT dsnLength = 0;
    SQLSMALLINT dsnDescriptionLength = 0;
    rc = SQLDataSources(
        Env_,
        SQL_FETCH_FIRST,
        dsn,
        sizeof(dsn),
        &dsnLength,
        dsnDescription,
        sizeof(dsnDescription),
        &dsnDescriptionLength);
    EXPECT_TRUE(IsOdbcSuccess(rc) || rc == SQL_NO_DATA) << "SQLDataSources returned " << rc;
}

TEST_F(OdbcCoreConformance, CoreConnectionAttributesFollowUnsetAndRoundTripSemantics) {
    const std::array<std::pair<SQLINTEGER, const char*>, 3> integerAttributes{{
        {SQL_ATTR_ACCESS_MODE, "SQL_ATTR_ACCESS_MODE"},
        {SQL_ATTR_ODBC_CURSORS, "SQL_ATTR_ODBC_CURSORS"},
        {SQL_ATTR_TRACE, "SQL_ATTR_TRACE"},
    }};
    for (const auto& [attribute, name] : integerAttributes) {
        SQLULEN value = 0;
        SQLINTEGER length = 0;
        const SQLRETURN rc = SQLGetConnectAttr(
            Dbc_, attribute, &value, sizeof(value), &length);
        EXPECT_TRUE(OdbcSuccess(rc, Dbc_, SQL_HANDLE_DBC)) << name;
    }

    SQLCHAR traceFile[512] = {};
    SQLINTEGER traceFileLength = 0;
    EXPECT_TRUE(OdbcSuccess(
        SQLGetConnectAttr(
            Dbc_, SQL_ATTR_TRACEFILE, traceFile, sizeof(traceFile), &traceFileLength),
        Dbc_, SQL_HANDLE_DBC));

    SQLPOINTER quietMode = reinterpret_cast<SQLPOINTER>(uintptr_t{1});
    EXPECT_EQ(
        SQLGetConnectAttr(Dbc_, SQL_ATTR_QUIET_MODE, &quietMode, sizeof(quietMode), nullptr),
        SQL_NO_DATA);
    SQLUINTEGER translateOption = 0;
    EXPECT_EQ(
        SQLGetConnectAttr(
            Dbc_, SQL_ATTR_TRANSLATE_OPTION, &translateOption, sizeof(translateOption), nullptr),
        SQL_NO_DATA);
    SQLCHAR translateLib[32] = {};
    EXPECT_EQ(
        SQLGetConnectAttr(
            Dbc_, SQL_ATTR_TRANSLATE_LIB, translateLib, sizeof(translateLib), nullptr),
        SQL_NO_DATA);

    quietMode = reinterpret_cast<SQLPOINTER>(uintptr_t{42});
    ASSERT_TRUE(OdbcSuccess(
        SQLSetConnectAttr(Dbc_, SQL_ATTR_QUIET_MODE, quietMode, 0),
        Dbc_, SQL_HANDLE_DBC));
    SQLPOINTER actualQuietMode = nullptr;
    SQLINTEGER quietModeLength = 0;
    ASSERT_TRUE(OdbcSuccess(
        SQLGetConnectAttr(
            Dbc_, SQL_ATTR_QUIET_MODE, &actualQuietMode, sizeof(actualQuietMode), &quietModeLength),
        Dbc_, SQL_HANDLE_DBC));
    EXPECT_EQ(actualQuietMode, quietMode);
    EXPECT_EQ(quietModeLength, static_cast<SQLINTEGER>(sizeof(SQLPOINTER)));

    constexpr SQLUINTEGER ExpectedTranslateOption = 17;
    ASSERT_TRUE(OdbcSuccess(
        SQLSetConnectAttr(
            Dbc_, SQL_ATTR_TRANSLATE_OPTION,
            reinterpret_cast<SQLPOINTER>(static_cast<uintptr_t>(ExpectedTranslateOption)), 0),
        Dbc_, SQL_HANDLE_DBC));
    SQLINTEGER translateOptionLength = 0;
    ASSERT_TRUE(OdbcSuccess(
        SQLGetConnectAttr(
            Dbc_, SQL_ATTR_TRANSLATE_OPTION, &translateOption, sizeof(translateOption),
            &translateOptionLength),
        Dbc_, SQL_HANDLE_DBC));
    EXPECT_EQ(translateOption, ExpectedTranslateOption);
    EXPECT_EQ(translateOptionLength, static_cast<SQLINTEGER>(sizeof(SQLUINTEGER)));

    ASSERT_TRUE(OdbcSuccess(
        SQLSetConnectAttr(
            Dbc_, SQL_ATTR_ACCESS_MODE, reinterpret_cast<SQLPOINTER>(SQL_MODE_READ_ONLY), 0),
        Dbc_,
        SQL_HANDLE_DBC));
    SQLULEN accessMode = 0;
    ASSERT_TRUE(OdbcSuccess(
        SQLGetConnectAttr(Dbc_, SQL_ATTR_ACCESS_MODE, &accessMode, sizeof(accessMode), nullptr),
        Dbc_,
        SQL_HANDLE_DBC));
    EXPECT_EQ(accessMode, static_cast<SQLULEN>(SQL_MODE_READ_ONLY));
}

TEST_F(OdbcCoreConformance, CoreStatementAttributeDefaultsAreReadable) {
    SQLHSTMT stmt = AllocStatement();
    ASSERT_NE(stmt, nullptr);

    const std::array<std::tuple<SQLINTEGER, SQLULEN, const char*>, 7> integerAttributes{{
        {SQL_ATTR_CURSOR_TYPE, SQL_CURSOR_FORWARD_ONLY, "SQL_ATTR_CURSOR_TYPE"},
        {SQL_ATTR_METADATA_ID, SQL_FALSE, "SQL_ATTR_METADATA_ID"},
        {SQL_ATTR_NOSCAN, SQL_NOSCAN_OFF, "SQL_ATTR_NOSCAN"},
        {SQL_ATTR_PARAM_BIND_TYPE, SQL_PARAM_BIND_BY_COLUMN, "SQL_ATTR_PARAM_BIND_TYPE"},
        {SQL_ATTR_PARAMSET_SIZE, 1, "SQL_ATTR_PARAMSET_SIZE"},
        {SQL_ATTR_ROW_ARRAY_SIZE, 1, "SQL_ATTR_ROW_ARRAY_SIZE"},
        {SQL_ATTR_ROW_BIND_TYPE, SQL_BIND_BY_COLUMN, "SQL_ATTR_ROW_BIND_TYPE"},
    }};
    for (const auto& [attribute, expected, name] : integerAttributes) {
        SQLULEN value = 0;
        const SQLRETURN rc = SQLGetStmtAttr(stmt, attribute, &value, sizeof(value), nullptr);
        EXPECT_TRUE(OdbcSuccess(rc, stmt, SQL_HANDLE_STMT)) << name;
        if (IsOdbcSuccess(rc)) {
            EXPECT_EQ(value, expected) << name;
        }
    }

    const std::array<std::pair<SQLINTEGER, const char*>, 7> pointerAttributes{{
        {SQL_ATTR_PARAM_BIND_OFFSET_PTR, "SQL_ATTR_PARAM_BIND_OFFSET_PTR"},
        {SQL_ATTR_PARAM_OPERATION_PTR, "SQL_ATTR_PARAM_OPERATION_PTR"},
        {SQL_ATTR_PARAM_STATUS_PTR, "SQL_ATTR_PARAM_STATUS_PTR"},
        {SQL_ATTR_PARAMS_PROCESSED_PTR, "SQL_ATTR_PARAMS_PROCESSED_PTR"},
        {SQL_ATTR_ROW_BIND_OFFSET_PTR, "SQL_ATTR_ROW_BIND_OFFSET_PTR"},
        {SQL_ATTR_ROW_STATUS_PTR, "SQL_ATTR_ROW_STATUS_PTR"},
        {SQL_ATTR_ROWS_FETCHED_PTR, "SQL_ATTR_ROWS_FETCHED_PTR"},
    }};
    for (const auto& [attribute, name] : pointerAttributes) {
        SQLPOINTER value = reinterpret_cast<SQLPOINTER>(uintptr_t{1});
        const SQLRETURN rc = SQLGetStmtAttr(stmt, attribute, &value, sizeof(value), nullptr);
        EXPECT_TRUE(OdbcSuccess(rc, stmt, SQL_HANDLE_STMT)) << name;
        if (IsOdbcSuccess(rc)) {
            EXPECT_EQ(value, nullptr) << name;
        }
    }

    for (const auto& [attribute, name] : std::array<std::pair<SQLINTEGER, const char*>, 4>{{
             {SQL_ATTR_APP_PARAM_DESC, "SQL_ATTR_APP_PARAM_DESC"},
             {SQL_ATTR_APP_ROW_DESC, "SQL_ATTR_APP_ROW_DESC"},
             {SQL_ATTR_IMP_PARAM_DESC, "SQL_ATTR_IMP_PARAM_DESC"},
             {SQL_ATTR_IMP_ROW_DESC, "SQL_ATTR_IMP_ROW_DESC"},
         }}) {
        SQLHDESC desc = SQL_NULL_HDESC;
        const SQLRETURN rc = SQLGetStmtAttr(stmt, attribute, &desc, sizeof(desc), nullptr);
        EXPECT_TRUE(OdbcSuccess(rc, stmt, SQL_HANDLE_STMT)) << name;
        if (IsOdbcSuccess(rc)) {
            EXPECT_NE(desc, nullptr) << name;
        }
    }

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}

TEST_F(OdbcCoreConformance, CoreStatementArrayAttributesCanBeConfigured) {
    SQLHSTMT stmt = AllocStatement();
    ASSERT_NE(stmt, nullptr);

    auto setAndGetInteger = [&](SQLINTEGER attribute, SQLULEN expected, const char* name) {
        SQLRETURN rc = SQLSetStmtAttr(
            stmt, attribute, reinterpret_cast<SQLPOINTER>(static_cast<uintptr_t>(expected)), 0);
        EXPECT_TRUE(OdbcSuccess(rc, stmt, SQL_HANDLE_STMT)) << name;
        if (!IsOdbcSuccess(rc)) {
            return;
        }
        SQLULEN actual = 0;
        rc = SQLGetStmtAttr(stmt, attribute, &actual, sizeof(actual), nullptr);
        EXPECT_TRUE(OdbcSuccess(rc, stmt, SQL_HANDLE_STMT)) << name;
        EXPECT_EQ(actual, expected) << name;
    };

    setAndGetInteger(SQL_ATTR_METADATA_ID, SQL_TRUE, "SQL_ATTR_METADATA_ID");
    setAndGetInteger(SQL_ATTR_NOSCAN, SQL_NOSCAN_ON, "SQL_ATTR_NOSCAN");
    setAndGetInteger(SQL_ATTR_PARAM_BIND_TYPE, 32, "SQL_ATTR_PARAM_BIND_TYPE");
    setAndGetInteger(SQL_ATTR_PARAMSET_SIZE, 2, "SQL_ATTR_PARAMSET_SIZE");
    setAndGetInteger(SQL_ATTR_ROW_ARRAY_SIZE, 2, "SQL_ATTR_ROW_ARRAY_SIZE");
    setAndGetInteger(SQL_ATTR_ROW_BIND_TYPE, 32, "SQL_ATTR_ROW_BIND_TYPE");

    SQLULEN offset = 8;
    SQLUSMALLINT operation[2] = {SQL_PARAM_PROCEED, SQL_PARAM_PROCEED};
    SQLUSMALLINT status[2] = {};
    SQLULEN processed = 0;
    SQLUSMALLINT rowStatus[2] = {};
    SQLULEN rowsFetched = 0;
    const std::array<std::tuple<SQLINTEGER, SQLPOINTER, const char*>, 7> pointerAttributes{{
        {SQL_ATTR_PARAM_BIND_OFFSET_PTR, &offset, "SQL_ATTR_PARAM_BIND_OFFSET_PTR"},
        {SQL_ATTR_PARAM_OPERATION_PTR, operation, "SQL_ATTR_PARAM_OPERATION_PTR"},
        {SQL_ATTR_PARAM_STATUS_PTR, status, "SQL_ATTR_PARAM_STATUS_PTR"},
        {SQL_ATTR_PARAMS_PROCESSED_PTR, &processed, "SQL_ATTR_PARAMS_PROCESSED_PTR"},
        {SQL_ATTR_ROW_BIND_OFFSET_PTR, &offset, "SQL_ATTR_ROW_BIND_OFFSET_PTR"},
        {SQL_ATTR_ROW_STATUS_PTR, rowStatus, "SQL_ATTR_ROW_STATUS_PTR"},
        {SQL_ATTR_ROWS_FETCHED_PTR, &rowsFetched, "SQL_ATTR_ROWS_FETCHED_PTR"},
    }};
    for (const auto& [attribute, expected, name] : pointerAttributes) {
        SQLRETURN rc = SQLSetStmtAttr(stmt, attribute, expected, 0);
        EXPECT_TRUE(OdbcSuccess(rc, stmt, SQL_HANDLE_STMT)) << name;
        if (!IsOdbcSuccess(rc)) {
            continue;
        }
        SQLPOINTER actual = nullptr;
        rc = SQLGetStmtAttr(stmt, attribute, &actual, sizeof(actual), nullptr);
        EXPECT_TRUE(OdbcSuccess(rc, stmt, SQL_HANDLE_STMT)) << name;
        EXPECT_EQ(actual, expected) << name;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}

TEST_F(OdbcCoreConformance, ApplicationDescriptorsCanBeAssigned) {
    SQLHSTMT stmt = AllocStatement();
    ASSERT_NE(stmt, nullptr);
    SQLHDESC ard = SQL_NULL_HDESC;
    SQLHDESC apd = SQL_NULL_HDESC;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DESC, Dbc_, &ard), SQL_SUCCESS);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DESC, Dbc_, &apd), SQL_SUCCESS);

    SQLRETURN rc = SQLSetStmtAttr(stmt, SQL_ATTR_APP_ROW_DESC, ard, 0);
    EXPECT_TRUE(OdbcSuccess(rc, stmt, SQL_HANDLE_STMT)) << "SQL_ATTR_APP_ROW_DESC";
    SQLHDESC actual = SQL_NULL_HDESC;
    if (IsOdbcSuccess(rc)) {
        ASSERT_TRUE(OdbcSuccess(
            SQLGetStmtAttr(stmt, SQL_ATTR_APP_ROW_DESC, &actual, sizeof(actual), nullptr),
            stmt,
            SQL_HANDLE_STMT));
        EXPECT_EQ(actual, ard);
    }

    rc = SQLSetStmtAttr(stmt, SQL_ATTR_APP_PARAM_DESC, apd, 0);
    EXPECT_TRUE(OdbcSuccess(rc, stmt, SQL_HANDLE_STMT)) << "SQL_ATTR_APP_PARAM_DESC";
    actual = SQL_NULL_HDESC;
    if (IsOdbcSuccess(rc)) {
        ASSERT_TRUE(OdbcSuccess(
            SQLGetStmtAttr(stmt, SQL_ATTR_APP_PARAM_DESC, &actual, sizeof(actual), nullptr),
            stmt,
            SQL_HANDLE_STMT));
        EXPECT_EQ(actual, apd);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLFreeHandle(SQL_HANDLE_DESC, ard);
    SQLFreeHandle(SQL_HANDLE_DESC, apd);
}

TEST_F(OdbcCoreConformance, ApplicationDescriptorsDriveBindingsAndDetachSafely) {
    SQLHSTMT first = AllocStatement();
    SQLHSTMT second = AllocStatement();
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    SQLHDESC apd = SQL_NULL_HDESC;
    SQLHDESC ard = SQL_NULL_HDESC;
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DESC, Dbc_, &apd), SQL_SUCCESS);
    ASSERT_EQ(SQLAllocHandle(SQL_HANDLE_DESC, Dbc_, &ard), SQL_SUCCESS);

    SQLINTEGER fallbackParameter = 1;
    ASSERT_TRUE(OdbcSuccess(
        SQLPrepare(first, reinterpret_cast<SQLCHAR*>(const_cast<char*>("SELECT ? AS value")), SQL_NTS),
        first, SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(
        SQLBindParameter(first, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
                         &fallbackParameter, 0, nullptr),
        first, SQL_HANDLE_STMT));
    SQLINTEGER descriptorParameter = 42;
    ASSERT_TRUE(OdbcSuccess(
        SQLSetDescRec(apd, 1, SQL_C_LONG, 0, sizeof(descriptorParameter), 0, 0,
                      &descriptorParameter, nullptr, nullptr),
        apd, SQL_HANDLE_DESC));
    ASSERT_TRUE(OdbcSuccess(
        SQLSetStmtAttr(first, SQL_ATTR_APP_PARAM_DESC, apd, 0), first, SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(SQLExecute(first), first, SQL_HANDLE_STMT));

    SQLINTEGER fallbackResult = 0;
    SQLLEN fallbackIndicator = 0;
    ASSERT_TRUE(OdbcSuccess(
        SQLBindCol(first, 1, SQL_C_LONG, &fallbackResult, 0, &fallbackIndicator),
        first, SQL_HANDLE_STMT));
    SQLINTEGER descriptorResult = 0;
    SQLLEN descriptorLength = -1;
    SQLLEN descriptorIndicator = -1;
    ASSERT_TRUE(OdbcSuccess(
        SQLSetDescRec(ard, 1, SQL_C_LONG, 0, 0, 0, 0,
                      &descriptorResult, &descriptorLength, &descriptorIndicator),
        ard, SQL_HANDLE_DESC));
    ASSERT_TRUE(OdbcSuccess(
        SQLSetStmtAttr(first, SQL_ATTR_APP_ROW_DESC, ard, 0), first, SQL_HANDLE_STMT));
    ASSERT_EQ(SQLFetch(first), SQL_SUCCESS);
    EXPECT_EQ(descriptorResult, 42);
    EXPECT_EQ(descriptorLength, static_cast<SQLLEN>(sizeof(descriptorResult)));
    EXPECT_EQ(descriptorIndicator, 0);
    EXPECT_EQ(fallbackResult, 0);

    ASSERT_TRUE(OdbcSuccess(
        SQLSetDescField(
            apd, 0, SQL_DESC_ARRAY_SIZE, reinterpret_cast<SQLPOINTER>(uintptr_t{2}), 0),
        apd, SQL_HANDLE_DESC));
    ASSERT_TRUE(OdbcSuccess(
        SQLSetStmtAttr(second, SQL_ATTR_APP_PARAM_DESC, apd, 0), second, SQL_HANDLE_STMT));
    SQLULEN size = 0;
    ASSERT_TRUE(OdbcSuccess(
        SQLGetStmtAttr(second, SQL_ATTR_PARAMSET_SIZE, &size, sizeof(size), nullptr),
        second, SQL_HANDLE_STMT));
    EXPECT_EQ(size, 2u);
    ASSERT_TRUE(OdbcSuccess(
        SQLSetStmtAttr(
            second, SQL_ATTR_PARAMSET_SIZE, reinterpret_cast<SQLPOINTER>(uintptr_t{3}), 0),
        second, SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(
        SQLGetDescField(apd, 0, SQL_DESC_ARRAY_SIZE, &size, 0, nullptr),
        apd, SQL_HANDLE_DESC));
    EXPECT_EQ(size, 3u);

    ASSERT_EQ(SQLFreeHandle(SQL_HANDLE_DESC, apd), SQL_SUCCESS);
    ASSERT_TRUE(OdbcSuccess(
        SQLGetStmtAttr(second, SQL_ATTR_PARAMSET_SIZE, &size, sizeof(size), nullptr),
        second, SQL_HANDLE_STMT));
    EXPECT_EQ(size, 1u);
    ASSERT_EQ(SQLFreeHandle(SQL_HANDLE_DESC, ard), SQL_SUCCESS);

    SQLFreeHandle(SQL_HANDLE_STMT, first);
    SQLFreeHandle(SQL_HANDLE_STMT, second);
}

TEST_F(OdbcCoreConformance, ParameterArraysRespectBindOffset) {
    SQLHSTMT stmt = AllocStatement();
    ASSERT_NE(stmt, nullptr);
    SQLExecDirect(
        stmt,
        reinterpret_cast<SQLCHAR*>(const_cast<char*>(
            "DROP TABLE IF EXISTS odbc_core_parameter_array")),
        SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    ASSERT_TRUE(OdbcSuccess(
        SQLExecDirect(
            stmt,
            reinterpret_cast<SQLCHAR*>(const_cast<char*>(
                "CREATE TABLE odbc_core_parameter_array (id Int32, PRIMARY KEY (id))")),
            SQL_NTS),
        stmt,
        SQL_HANDLE_STMT));
    SQLFreeStmt(stmt, SQL_CLOSE);

    struct TParameterRow {
        SQLINTEGER Id;
        SQLLEN Indicator;
    };
    std::array<TParameterRow, 3> parameters{{
        {0, 0}, // deliberately skipped by SQL_ATTR_PARAM_BIND_OFFSET_PTR
        {101, 0},
        {102, 0},
    }};
    SQLULEN bindOffset = sizeof(TParameterRow);
    SQLUSMALLINT operations[2] = {SQL_PARAM_PROCEED, SQL_PARAM_PROCEED};
    SQLUSMALLINT statuses[2] = {};
    SQLULEN processed = 0;

    ASSERT_TRUE(OdbcSuccess(
        SQLSetStmtAttr(
            stmt,
            SQL_ATTR_PARAM_BIND_TYPE,
            reinterpret_cast<SQLPOINTER>(sizeof(TParameterRow)),
            0),
        stmt,
        SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(
        SQLSetStmtAttr(stmt, SQL_ATTR_PARAM_BIND_OFFSET_PTR, &bindOffset, 0),
        stmt,
        SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(
        SQLSetStmtAttr(
            stmt, SQL_ATTR_PARAMSET_SIZE, reinterpret_cast<SQLPOINTER>(uintptr_t{2}), 0),
        stmt,
        SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(
        SQLSetStmtAttr(stmt, SQL_ATTR_PARAM_OPERATION_PTR, operations, 0),
        stmt,
        SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(
        SQLSetStmtAttr(stmt, SQL_ATTR_PARAM_STATUS_PTR, statuses, 0),
        stmt,
        SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(
        SQLSetStmtAttr(stmt, SQL_ATTR_PARAMS_PROCESSED_PTR, &processed, 0),
        stmt,
        SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(
        SQLPrepare(
            stmt,
            reinterpret_cast<SQLCHAR*>(const_cast<char*>(
                "UPSERT INTO odbc_core_parameter_array (id) VALUES (?)")),
            SQL_NTS),
        stmt,
        SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(
        SQLBindParameter(
            stmt,
            1,
            SQL_PARAM_INPUT,
            SQL_C_LONG,
            SQL_INTEGER,
            0,
            0,
            &parameters[0].Id,
            0,
            &parameters[0].Indicator),
        stmt,
        SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(SQLExecute(stmt), stmt, SQL_HANDLE_STMT));
    EXPECT_EQ(processed, 2u);
    for (SQLUSMALLINT status : statuses) {
        EXPECT_TRUE(status == SQL_PARAM_SUCCESS || status == SQL_PARAM_SUCCESS_WITH_INFO)
            << "parameter status " << status;
    }

    SQLFreeStmt(stmt, SQL_CLOSE);
    ASSERT_TRUE(OdbcSuccess(
        SQLExecDirect(
            stmt,
            reinterpret_cast<SQLCHAR*>(const_cast<char*>(
                "SELECT COUNT(*) FROM odbc_core_parameter_array")),
            SQL_NTS),
        stmt,
        SQL_HANDLE_STMT));
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    SQLBIGINT count = 0;
    ASSERT_TRUE(OdbcSuccess(
        SQLGetData(stmt, 1, SQL_C_SBIGINT, &count, sizeof(count), nullptr),
        stmt,
        SQL_HANDLE_STMT));
    EXPECT_EQ(count, 2);

    SQLFreeStmt(stmt, SQL_CLOSE);
    SQLExecDirect(
        stmt,
        reinterpret_cast<SQLCHAR*>(const_cast<char*>(
            "DROP TABLE odbc_core_parameter_array")),
        SQL_NTS);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}

TEST_F(OdbcCoreConformance, RowArraysRespectBindOffset) {
    SQLHSTMT stmt = AllocStatement();
    ASSERT_NE(stmt, nullptr);
    SQLExecDirect(
        stmt,
        reinterpret_cast<SQLCHAR*>(const_cast<char*>(
            "DROP TABLE IF EXISTS odbc_core_row_array")),
        SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    ASSERT_TRUE(OdbcSuccess(
        SQLExecDirect(
            stmt,
            reinterpret_cast<SQLCHAR*>(const_cast<char*>(
                "CREATE TABLE odbc_core_row_array (id Int32, PRIMARY KEY (id))")),
            SQL_NTS),
        stmt,
        SQL_HANDLE_STMT));
    SQLFreeStmt(stmt, SQL_CLOSE);
    for (const char* sql : {
             "UPSERT INTO odbc_core_row_array (id) VALUES (1)",
             "UPSERT INTO odbc_core_row_array (id) VALUES (2)",
             "UPSERT INTO odbc_core_row_array (id) VALUES (3)",
         }) {
        ASSERT_TRUE(OdbcSuccess(
            SQLExecDirect(stmt, reinterpret_cast<SQLCHAR*>(const_cast<char*>(sql)), SQL_NTS),
            stmt,
            SQL_HANDLE_STMT));
        SQLFreeStmt(stmt, SQL_CLOSE);
    }

    struct TRow {
        SQLINTEGER Id;
        SQLLEN Indicator;
    };
    std::array<TRow, 3> rows{};
    SQLULEN bindOffset = sizeof(TRow);
    SQLUSMALLINT statuses[2] = {};
    SQLULEN fetched = 0;
    ASSERT_TRUE(OdbcSuccess(
        SQLSetStmtAttr(
            stmt,
            SQL_ATTR_ROW_BIND_TYPE,
            reinterpret_cast<SQLPOINTER>(sizeof(TRow)),
            0),
        stmt,
        SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(
        SQLSetStmtAttr(stmt, SQL_ATTR_ROW_BIND_OFFSET_PTR, &bindOffset, 0),
        stmt,
        SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(
        SQLSetStmtAttr(
            stmt, SQL_ATTR_ROW_ARRAY_SIZE, reinterpret_cast<SQLPOINTER>(uintptr_t{2}), 0),
        stmt,
        SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(
        SQLSetStmtAttr(stmt, SQL_ATTR_ROW_STATUS_PTR, statuses, 0),
        stmt,
        SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(
        SQLSetStmtAttr(stmt, SQL_ATTR_ROWS_FETCHED_PTR, &fetched, 0),
        stmt,
        SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(
        SQLExecDirect(
            stmt,
            reinterpret_cast<SQLCHAR*>(const_cast<char*>(
                "SELECT id FROM odbc_core_row_array ORDER BY id")),
            SQL_NTS),
        stmt,
        SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(
        SQLBindCol(stmt, 1, SQL_C_LONG, &rows[0].Id, 0, &rows[0].Indicator),
        stmt,
        SQL_HANDLE_STMT));

    ASSERT_TRUE(OdbcSuccess(SQLFetchScroll(stmt, SQL_FETCH_NEXT, 0), stmt, SQL_HANDLE_STMT));
    ASSERT_EQ(fetched, 2u);
    EXPECT_EQ(rows[1].Id, 1);
    EXPECT_EQ(rows[2].Id, 2);
    rows[1].Id = 0;
    ASSERT_TRUE(OdbcSuccess(SQLFetchScroll(stmt, SQL_FETCH_NEXT, 0), stmt, SQL_HANDLE_STMT));
    ASSERT_EQ(fetched, 1u);
    EXPECT_EQ(rows[1].Id, 3);

    SQLFreeStmt(stmt, SQL_CLOSE);
    SQLExecDirect(
        stmt,
        reinterpret_cast<SQLCHAR*>(const_cast<char*>(
            "DROP TABLE odbc_core_row_array")),
        SQL_NTS);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}

TEST_F(OdbcCoreConformance, CoreDescriptorHeaderFieldsAreReadable) {
    SQLHSTMT stmt = AllocStatement();
    ASSERT_NE(stmt, nullptr);
    ASSERT_TRUE(OdbcSuccess(
        SQLPrepare(stmt, reinterpret_cast<SQLCHAR*>(const_cast<char*>("SELECT ? AS value")), SQL_NTS),
        stmt,
        SQL_HANDLE_STMT));
    SQLINTEGER parameter = 7;
    ASSERT_TRUE(OdbcSuccess(
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &parameter, 0, nullptr),
        stmt,
        SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(SQLExecute(stmt), stmt, SQL_HANDLE_STMT));
    SQLINTEGER result = 0;
    SQLLEN indicator = 0;
    ASSERT_TRUE(OdbcSuccess(
        SQLBindCol(stmt, 1, SQL_C_LONG, &result, 0, &indicator),
        stmt,
        SQL_HANDLE_STMT));

    SQLHDESC ard = SQL_NULL_HDESC;
    SQLHDESC apd = SQL_NULL_HDESC;
    SQLHDESC ird = SQL_NULL_HDESC;
    SQLHDESC ipd = SQL_NULL_HDESC;
    ASSERT_TRUE(OdbcSuccess(SQLGetStmtAttr(stmt, SQL_ATTR_APP_ROW_DESC, &ard, sizeof(ard), nullptr), stmt, SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(SQLGetStmtAttr(stmt, SQL_ATTR_APP_PARAM_DESC, &apd, sizeof(apd), nullptr), stmt, SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(SQLGetStmtAttr(stmt, SQL_ATTR_IMP_ROW_DESC, &ird, sizeof(ird), nullptr), stmt, SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(SQLGetStmtAttr(stmt, SQL_ATTR_IMP_PARAM_DESC, &ipd, sizeof(ipd), nullptr), stmt, SQL_HANDLE_STMT));

    for (SQLHDESC desc : {ard, apd, ird, ipd}) {
        ExpectDescriptorField(desc, 0, SQL_DESC_ALLOC_TYPE, "SQL_DESC_ALLOC_TYPE");
        ExpectDescriptorField(desc, 0, SQL_DESC_COUNT, "SQL_DESC_COUNT");
    }
    for (SQLHDESC desc : {ard, apd}) {
        ExpectDescriptorField(desc, 0, SQL_DESC_ARRAY_SIZE, "SQL_DESC_ARRAY_SIZE");
        ExpectDescriptorField(desc, 0, SQL_DESC_BIND_OFFSET_PTR, "SQL_DESC_BIND_OFFSET_PTR");
        ExpectDescriptorField(desc, 0, SQL_DESC_BIND_TYPE, "SQL_DESC_BIND_TYPE");
    }
    for (SQLHDESC desc : {apd, ipd, ird}) {
        ExpectDescriptorField(desc, 0, SQL_DESC_ARRAY_STATUS_PTR, "SQL_DESC_ARRAY_STATUS_PTR");
    }
    for (SQLHDESC desc : {ipd, ird}) {
        ExpectDescriptorField(desc, 0, SQL_DESC_ROWS_PROCESSED_PTR, "SQL_DESC_ROWS_PROCESSED_PTR");
    }

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}

TEST_F(OdbcCoreConformance, CoreDescriptorRecordFieldsAreReadable) {
    SQLHSTMT stmt = AllocStatement();
    ASSERT_NE(stmt, nullptr);
    ASSERT_TRUE(OdbcSuccess(
        SQLPrepare(stmt, reinterpret_cast<SQLCHAR*>(const_cast<char*>("SELECT ? AS value")), SQL_NTS),
        stmt,
        SQL_HANDLE_STMT));
    SQLINTEGER parameter = 7;
    SQLLEN parameterIndicator = 0;
    ASSERT_TRUE(OdbcSuccess(
        SQLBindParameter(
            stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
            &parameter, 0, &parameterIndicator),
        stmt,
        SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(SQLExecute(stmt), stmt, SQL_HANDLE_STMT));
    SQLINTEGER result = 0;
    SQLLEN resultIndicator = 0;
    ASSERT_TRUE(OdbcSuccess(
        SQLBindCol(stmt, 1, SQL_C_LONG, &result, 0, &resultIndicator),
        stmt,
        SQL_HANDLE_STMT));

    SQLHDESC ard = SQL_NULL_HDESC;
    SQLHDESC apd = SQL_NULL_HDESC;
    SQLHDESC ird = SQL_NULL_HDESC;
    SQLHDESC ipd = SQL_NULL_HDESC;
    ASSERT_TRUE(OdbcSuccess(SQLGetStmtAttr(stmt, SQL_ATTR_APP_ROW_DESC, &ard, sizeof(ard), nullptr), stmt, SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(SQLGetStmtAttr(stmt, SQL_ATTR_APP_PARAM_DESC, &apd, sizeof(apd), nullptr), stmt, SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(SQLGetStmtAttr(stmt, SQL_ATTR_IMP_ROW_DESC, &ird, sizeof(ird), nullptr), stmt, SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(SQLGetStmtAttr(stmt, SQL_ATTR_IMP_PARAM_DESC, &ipd, sizeof(ipd), nullptr), stmt, SQL_HANDLE_STMT));

    for (const auto& [field, name] : std::array<std::pair<SQLSMALLINT, const char*>, 6>{{
             {SQL_DESC_BASE_COLUMN_NAME, "SQL_DESC_BASE_COLUMN_NAME"},
             {SQL_DESC_LITERAL_PREFIX, "SQL_DESC_LITERAL_PREFIX"},
             {SQL_DESC_LITERAL_SUFFIX, "SQL_DESC_LITERAL_SUFFIX"},
             {SQL_DESC_LOCAL_TYPE_NAME, "SQL_DESC_LOCAL_TYPE_NAME"},
             {SQL_DESC_NAME, "SQL_DESC_NAME"},
             {SQL_DESC_TYPE_NAME, "SQL_DESC_TYPE_NAME"},
         }}) {
        ExpectDescriptorField(ird, 1, field, name, true);
    }
    for (const auto& [field, name] : std::array<std::pair<SQLSMALLINT, const char*>, 14>{{
             {SQL_DESC_CASE_SENSITIVE, "SQL_DESC_CASE_SENSITIVE"},
             {SQL_DESC_CONCISE_TYPE, "SQL_DESC_CONCISE_TYPE"},
             {SQL_DESC_DISPLAY_SIZE, "SQL_DESC_DISPLAY_SIZE"},
             {SQL_DESC_FIXED_PREC_SCALE, "SQL_DESC_FIXED_PREC_SCALE"},
             {SQL_DESC_LENGTH, "SQL_DESC_LENGTH"},
             {SQL_DESC_NULLABLE, "SQL_DESC_NULLABLE"},
             {SQL_DESC_OCTET_LENGTH, "SQL_DESC_OCTET_LENGTH"},
             {SQL_DESC_PRECISION, "SQL_DESC_PRECISION"},
             {SQL_DESC_SCALE, "SQL_DESC_SCALE"},
             {SQL_DESC_SEARCHABLE, "SQL_DESC_SEARCHABLE"},
             {SQL_DESC_TYPE, "SQL_DESC_TYPE"},
             {SQL_DESC_UNNAMED, "SQL_DESC_UNNAMED"},
             {SQL_DESC_UNSIGNED, "SQL_DESC_UNSIGNED"},
             {SQL_DESC_UPDATABLE, "SQL_DESC_UPDATABLE"},
         }}) {
        ExpectDescriptorField(ird, 1, field, name);
    }

    for (const auto& [field, name] : std::array<std::pair<SQLSMALLINT, const char*>, 3>{{
             {SQL_DESC_DATA_PTR, "SQL_DESC_DATA_PTR"},
             {SQL_DESC_INDICATOR_PTR, "SQL_DESC_INDICATOR_PTR"},
             {SQL_DESC_OCTET_LENGTH_PTR, "SQL_DESC_OCTET_LENGTH_PTR"},
         }}) {
        ExpectDescriptorField(ard, 1, field, name);
        ExpectDescriptorField(apd, 1, field, name);
    }
    ExpectDescriptorField(ipd, 1, SQL_DESC_PARAMETER_TYPE, "SQL_DESC_PARAMETER_TYPE");

    SQLFreeStmt(stmt, SQL_CLOSE);
    ASSERT_TRUE(OdbcSuccess(
        SQLExecDirect(
            stmt,
            reinterpret_cast<SQLCHAR*>(const_cast<char*>(
                "SELECT CAST('2024-06-15T14:30:00Z' AS Datetime) AS value")),
            SQL_NTS),
        stmt,
        SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(
        SQLGetStmtAttr(stmt, SQL_ATTR_IMP_ROW_DESC, &ird, sizeof(ird), nullptr),
        stmt,
        SQL_HANDLE_STMT));
    ExpectDescriptorField(
        ird,
        1,
        SQL_DESC_DATETIME_INTERVAL_CODE,
        "SQL_DESC_DATETIME_INTERVAL_CODE");

    SQLSMALLINT conciseType = 0;
    EXPECT_TRUE(OdbcSuccess(
        SQLGetDescRec(ird, 1, nullptr, 0, nullptr, &conciseType, nullptr,
                      nullptr, nullptr, nullptr, nullptr),
        ird, SQL_HANDLE_DESC));
    EXPECT_EQ(SQLGetDescField(
        ird, 2, SQL_DESC_CONCISE_TYPE, &conciseType, 0, nullptr), SQL_NO_DATA);

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}

TEST_F(OdbcCoreConformance, ReportsConservativeCapabilitiesAndTransactionSemantics) {
    SQLUINTEGER value = 0;
    ASSERT_TRUE(OdbcSuccess(SQLGetInfo(
        Dbc_, SQL_PARAM_ARRAY_ROW_COUNTS, &value, sizeof(value), nullptr), Dbc_, SQL_HANDLE_DBC));
    EXPECT_EQ(value, static_cast<SQLUINTEGER>(SQL_PARC_NO_BATCH));
    ASSERT_TRUE(OdbcSuccess(SQLGetInfo(
        Dbc_, SQL_PARAM_ARRAY_SELECTS, &value, sizeof(value), nullptr), Dbc_, SQL_HANDLE_DBC));
    EXPECT_EQ(value, static_cast<SQLUINTEGER>(SQL_PAS_NO_SELECT));

    SQLUSMALLINT txnCapable = 0;
    ASSERT_TRUE(OdbcSuccess(SQLGetInfo(
        Dbc_, SQL_TXN_CAPABLE, &txnCapable, sizeof(txnCapable), nullptr), Dbc_, SQL_HANDLE_DBC));
    EXPECT_EQ(txnCapable, static_cast<SQLUSMALLINT>(SQL_TC_DML));
    EXPECT_EQ(SQLEndTran(SQL_HANDLE_DBC, Dbc_, SQL_COMMIT), SQL_SUCCESS);
    EXPECT_EQ(SQLEndTran(SQL_HANDLE_DBC, Dbc_, 99), SQL_ERROR);
    SQLCHAR state[6] = {};
    ASSERT_TRUE(IsOdbcSuccess(SQLGetDiagRec(
        SQL_HANDLE_DBC, Dbc_, 1, state, nullptr, nullptr, 0, nullptr)));
    EXPECT_STREQ(reinterpret_cast<char*>(state), "HY012");
}

TEST_F(OdbcCoreConformance, RejectsSelectParameterArrays) {
    SQLHSTMT stmt = AllocStatement();
    SQLINTEGER values[2] = {1, 2};
    SQLLEN indicators[2] = {0, 0};
    ASSERT_TRUE(OdbcSuccess(SQLSetStmtAttr(
        stmt, SQL_ATTR_PARAMSET_SIZE, reinterpret_cast<SQLPOINTER>(uintptr_t{2}), 0),
        stmt, SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(SQLPrepare(
        stmt, reinterpret_cast<SQLCHAR*>(const_cast<char*>("SELECT ?")), SQL_NTS),
        stmt, SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(SQLBindParameter(
        stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
        values, 0, indicators), stmt, SQL_HANDLE_STMT));
    EXPECT_EQ(SQLExecute(stmt), SQL_ERROR);
    EXPECT_NE(GetOdbcError(stmt, SQL_HANDLE_STMT).find("HYC00"), std::string::npos);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}

TEST_F(OdbcCoreConformance, BindingsMayPrecedeExecDirect) {
    SQLHSTMT stmt = AllocStatement();
    SQLINTEGER parameter = 77;
    ASSERT_TRUE(OdbcSuccess(SQLBindParameter(
        stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
        &parameter, 0, nullptr), stmt, SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(SQLExecDirect(
        stmt, reinterpret_cast<SQLCHAR*>(const_cast<char*>("SELECT ?")), SQL_NTS),
        stmt, SQL_HANDLE_STMT));
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    SQLINTEGER result = 0;
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_LONG, &result, 0, nullptr), SQL_SUCCESS);
    EXPECT_EQ(result, parameter);

    ASSERT_EQ(SQLFreeStmt(stmt, SQL_CLOSE), SQL_SUCCESS);
    ASSERT_EQ(SQLFreeStmt(stmt, SQL_RESET_PARAMS), SQL_SUCCESS);
    SQLLEN nullIndicator = SQL_NULL_DATA;
    ASSERT_TRUE(OdbcSuccess(SQLBindParameter(
        stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
        nullptr, 0, &nullIndicator), stmt, SQL_HANDLE_STMT));
    ASSERT_TRUE(OdbcSuccess(SQLExecDirect(
        stmt, reinterpret_cast<SQLCHAR*>(const_cast<char*>("SELECT ? IS NULL")), SQL_NTS),
        stmt, SQL_HANDLE_STMT));
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);
    SQLCHAR isNull = 0;
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_BIT, &isNull, 0, nullptr), SQL_SUCCESS);
    EXPECT_EQ(isNull, 1);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}

TEST_F(OdbcCoreConformance, GetDataConvertsNumbersAndContinuesText) {
    SQLHSTMT stmt = AllocStatement();
    ASSERT_TRUE(OdbcSuccess(SQLExecDirect(
        stmt, reinterpret_cast<SQLCHAR*>(const_cast<char*>(
            "SELECT 123 AS number, 'abcdef' AS text")), SQL_NTS),
        stmt, SQL_HANDLE_STMT));
    ASSERT_EQ(SQLFetch(stmt), SQL_SUCCESS);

    char numeric[16] = {};
    SQLLEN length = 0;
    EXPECT_EQ(SQLGetData(stmt, 1, SQL_C_CHAR, numeric, -1, &length), SQL_ERROR);
    EXPECT_NE(GetOdbcError(stmt, SQL_HANDLE_STMT).find("HY090"), std::string::npos);
    ASSERT_EQ(SQLGetData(stmt, 1, SQL_C_CHAR, numeric, sizeof(numeric), &length), SQL_SUCCESS);
    EXPECT_STREQ(numeric, "123");

    char chunk[4] = {};
    EXPECT_EQ(SQLGetData(stmt, 2, SQL_C_CHAR, chunk, sizeof(chunk), &length), SQL_SUCCESS_WITH_INFO);
    EXPECT_STREQ(chunk, "abc");
    EXPECT_EQ(length, 6);
    ASSERT_EQ(SQLGetData(stmt, 2, SQL_C_CHAR, chunk, sizeof(chunk), &length), SQL_SUCCESS);
    EXPECT_STREQ(chunk, "def");
    EXPECT_EQ(length, 3);
    EXPECT_EQ(SQLGetData(stmt, 2, SQL_C_CHAR, chunk, sizeof(chunk), &length), SQL_NO_DATA);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}

TEST_F(OdbcCoreConformance, DiagnosticReadsAreStableAndSuccessClearsThem) {
    SQLHSTMT stmt = AllocStatement();
    ASSERT_EQ(SQLExecDirect(
        stmt, reinterpret_cast<SQLCHAR*>(const_cast<char*>("invalid syntax")), SQL_NTS), SQL_ERROR);
    SQLCHAR first[6] = {};
    SQLCHAR second[6] = {};
    ASSERT_EQ(SQLGetDiagRec(
        SQL_HANDLE_STMT, stmt, 1, first, nullptr, nullptr, 0, nullptr), SQL_SUCCESS);
    ASSERT_EQ(SQLGetDiagRec(
        SQL_HANDLE_STMT, stmt, 1, second, nullptr, nullptr, 0, nullptr), SQL_SUCCESS);
    EXPECT_STREQ(reinterpret_cast<char*>(first), reinterpret_cast<char*>(second));

    SQLRETURN prior = SQL_SUCCESS;
    ASSERT_EQ(SQLGetDiagField(
        SQL_HANDLE_STMT, stmt, 0, SQL_DIAG_RETURNCODE, &prior, 0, nullptr), SQL_SUCCESS);
    EXPECT_EQ(prior, SQL_ERROR);
    EXPECT_EQ(SQLPrepare(
        stmt, reinterpret_cast<SQLCHAR*>(const_cast<char*>("SELECT 1")), SQL_NTS), SQL_SUCCESS);
    EXPECT_EQ(SQLGetDiagRec(
        SQL_HANDLE_STMT, stmt, 1, first, nullptr, nullptr, 0, nullptr), SQL_NO_DATA);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}

TEST_F(OdbcCoreConformance, CoreCatalogFunctionsReturnStandardResultShapes) {
    SQLHSTMT stmt = AllocStatement();
    ASSERT_NE(stmt, nullptr);
    const char* table = "/local/odbc_core_conformance_catalog";
    SQLExecDirect(
        stmt,
        reinterpret_cast<SQLCHAR*>(const_cast<char*>("DROP TABLE IF EXISTS odbc_core_conformance_catalog")),
        SQL_NTS);
    SQLFreeStmt(stmt, SQL_CLOSE);
    ASSERT_TRUE(OdbcSuccess(
        SQLExecDirect(
            stmt,
            reinterpret_cast<SQLCHAR*>(const_cast<char*>(
                "CREATE TABLE odbc_core_conformance_catalog (id Int32, value Text, PRIMARY KEY (id))")),
            SQL_NTS),
        stmt,
        SQL_HANDLE_STMT));
    SQLFreeStmt(stmt, SQL_CLOSE);

    auto expectColumns = [&](SQLRETURN rc, SQLSMALLINT expected, const char* name) {
        ASSERT_TRUE(OdbcSuccess(rc, stmt, SQL_HANDLE_STMT)) << name;
        SQLSMALLINT actual = 0;
        ASSERT_TRUE(OdbcSuccess(SQLNumResultCols(stmt, &actual), stmt, SQL_HANDLE_STMT)) << name;
        EXPECT_EQ(actual, expected) << name;
        SQLFreeStmt(stmt, SQL_CLOSE);
    };

    expectColumns(
        SQLTables(stmt, nullptr, 0, nullptr, 0,
                  reinterpret_cast<SQLCHAR*>(const_cast<char*>(table)), SQL_NTS,
                  reinterpret_cast<SQLCHAR*>(const_cast<char*>("TABLE")), SQL_NTS),
        5,
        "SQLTables");
    expectColumns(
        SQLColumns(stmt, nullptr, 0, nullptr, 0,
                   reinterpret_cast<SQLCHAR*>(const_cast<char*>(table)), SQL_NTS,
                   nullptr, 0),
        18,
        "SQLColumns");
    expectColumns(SQLGetTypeInfo(stmt, SQL_ALL_TYPES), 19, "SQLGetTypeInfo");
    expectColumns(
        SQLStatistics(stmt, nullptr, 0, nullptr, 0,
                      reinterpret_cast<SQLCHAR*>(const_cast<char*>(table)), SQL_NTS,
                      SQL_INDEX_ALL, SQL_ENSURE),
        13,
        "SQLStatistics");
    expectColumns(
        SQLSpecialColumns(stmt, SQL_BEST_ROWID, nullptr, 0, nullptr, 0,
                          reinterpret_cast<SQLCHAR*>(const_cast<char*>(table)), SQL_NTS,
                          SQL_SCOPE_SESSION, SQL_NULLABLE),
        8,
        "SQLSpecialColumns");

    SQLExecDirect(
        stmt,
        reinterpret_cast<SQLCHAR*>(const_cast<char*>("DROP TABLE odbc_core_conformance_catalog")),
        SQL_NTS);
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}
