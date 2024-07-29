#include "pagination.h"

#include <ydb-cpp-sdk/util/string/cast.h>
#include <ydb-cpp-sdk/json_value/ydb_json_value.h>

#include <filesystem>

void ThrowOnError(const TStatus& status) {
    if (!status.IsSuccess()) {
        throw TYdbErrorException(status) << status;
    }
}

static std::string JoinPath(const std::string& basePath, const std::string& path) {
    if (basePath.empty()) {
        return path;
    }

    std::filesystem::path prefixPathSplit(basePath);
    prefixPathSplit /= path;

    return prefixPathSplit;
}

//! Creates sample table with CrateTable API.
void CreateTable(TTableClient client, const std::string& path) {
    ThrowOnError(client.RetryOperationSync([path](TSession session) {
        auto schoolsDesc = TTableBuilder()
            .AddNullableColumn("city", EPrimitiveType::Utf8)
            .AddNullableColumn("number", EPrimitiveType::Uint32)
            .AddNullableColumn("address", EPrimitiveType::Utf8)
            .SetPrimaryKeyColumns({ "city", "number" })
            .Build();

        return session.CreateTable(JoinPath(path, "schools"), std::move(schoolsDesc)).GetValueSync();
    }));
}

//! Fills sample tables with data in single parameterized data query.
TStatus FillTableDataTransaction(TSession& session, const std::string& path) {
    auto query = std::format(R"(
        --!syntax_v1
        PRAGMA TablePathPrefix("{}");

        DECLARE $schoolsData AS List<Struct<
            city: Utf8,
            number: Uint32,
            address: Utf8>>;

        REPLACE INTO schools
        SELECT
            city,
            number,
            address
        FROM AS_TABLE($schoolsData);
    )", path);

    auto params = GetTablesDataParams();

    return session.ExecuteDataQuery(
        query,
        TTxControl::BeginTx(TTxSettings::SerializableRW()).CommitTx(),
        std::move(params)).GetValueSync();
}

//! Shows usage of query paging.
static TStatus SelectPagingTransaction(TSession& session, const std::string& path,
    uint64_t pageLimit, const std::string& lastCity, uint32_t lastNumber, std::optional<TResultSet>& resultSet)
{
    auto query = std::format(R"(
        --!syntax_v1
        PRAGMA TablePathPrefix("{}");

        DECLARE $limit AS Uint64;
        DECLARE $lastCity AS Utf8;
        DECLARE $lastNumber AS Uint32;

        $part1 = (
            SELECT * FROM schools
            WHERE city = $lastCity AND number > $lastNumber
            ORDER BY city, number LIMIT $limit
        );

        $part2 = (
            SELECT * FROM schools
            WHERE city > $lastCity
            ORDER BY city, number LIMIT $limit
        );

        $union = (
            SELECT * FROM $part1
            UNION ALL
            SELECT * FROM $part2
        );

        SELECT * FROM $union
        ORDER BY city, number LIMIT $limit;
    )", path);

    auto params = session.GetParamsBuilder()
        .AddParam("$limit")
            .Uint64(pageLimit)
            .Build()
        .AddParam("$lastCity")
            .Utf8(lastCity)
            .Build()
        .AddParam("$lastNumber")
            .Uint32(lastNumber)
            .Build()
        .Build();

    auto result = session.ExecuteDataQuery(
        query,
        TTxControl::BeginTx(TTxSettings::SerializableRW()).CommitTx(),
        std::move(params)).GetValueSync();

    if (result.IsSuccess()) {
        resultSet = result.GetResultSet(0);
    }

    return result;
}

std::string SelectPaging(TTableClient client, const std::string& path, uint64_t pageLimit, std::string& lastCity, uint32_t& lastNumber) {
    std::optional<TResultSet> resultSet;
    ThrowOnError(client.RetryOperationSync([path, pageLimit, &lastCity, lastNumber, &resultSet](TSession session) {
        return SelectPagingTransaction(session, path, pageLimit, lastCity, lastNumber, resultSet);
    }));

    TResultSetParser parser(*resultSet);

    if (!parser.TryNextRow()) {
        return "";
    }
    do {
        lastCity = parser.ColumnParser("city").GetOptionalUtf8().value();
        lastNumber = parser.ColumnParser("number").GetOptionalUint32().value();
    } while (parser.TryNextRow());
    return FormatResultSetJson(resultSet.value(), EBinaryStringEncoding::Unicode);
}

