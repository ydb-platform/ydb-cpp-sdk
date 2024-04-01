#include "pagination.h"

#include <util/folder/pathsplit.h>

using namespace NYdb;
using namespace NYdb::NTable;

const ui32 MaxPages = 10;

class TYdbErrorException : public yexception {
public:
    TYdbErrorException(const TStatus& status)
        : Status(status) {}

    TStatus Status;
};

static void ThrowOnError(const TStatus& status) {
    if (!status.IsSuccess()) {
        throw TYdbErrorException(status) << status;
    }
}

static void PrintStatus(const TStatus& status) {
    std::cerr << "Status: " << ToString(status.GetStatus()) << std::endl;
    std::cerr << status.GetIssues().ToString();
}

static std::string JoinPath(const std::string& basePath, const std::string& path) {
    if (basePath.empty()) {
        return path;
    }

    TPathSplitUnix prefixPathSplit(basePath);
    prefixPathSplit.AppendComponent(path);

    return prefixPathSplit.Reconstruct();
}

//! Creates sample table with CrateTable API.
static void CreateTable(TTableClient client, const std::string& path) {
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
static TStatus FillTableDataTransaction(TSession& session, const std::string& path) {
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
    ui64 pageLimit, const std::string& lastCity, ui32 lastNumber, std::optional<TResultSet>& resultSet)
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

bool SelectPaging(TTableClient client, const std::string& path, ui64 pageLimit, std::string& lastCity, ui32& lastNumber) {
    std::optional<TResultSet> resultSet;
    ThrowOnError(client.RetryOperationSync([path, pageLimit, &lastCity, lastNumber, &resultSet](TSession session) {
        return SelectPagingTransaction(session, path, pageLimit, lastCity, lastNumber, resultSet);
    }));

    TResultSetParser parser(*resultSet);

    if (!parser.TryNextRow()) {
        return false;
    }
    do {
        lastCity = parser.ColumnParser("city").GetOptionalUtf8().value();
        lastNumber = parser.ColumnParser("number").GetOptionalUint32().value();
        std::cout << lastCity << ", Школа №" << lastNumber << ", Адрес: " << ToString(parser.ColumnParser("address").GetOptionalUtf8()) << std::endl;
    } while (parser.TryNextRow());
    return true;
}

bool Run(const TDriver& driver, const std::string& path) {
    TTableClient client(driver);

    try {
        CreateTable(client, path);

        ThrowOnError(client.RetryOperationSync([path](TSession session) {
            return FillTableDataTransaction(session, path);
        }));

        ui64 limit = 3;
        std::string lastCity;
        ui32 lastNumber = 0;
        ui32 page = 0;
        bool pageNotEmpty = true;

        std::cout << "> Pagination, Limit=" << limit << std::endl;

        // show first MaxPages=10 pages:
        while (pageNotEmpty && page <= MaxPages) {
            ++page;
            std::cout << "> Page " << page << ":" << std::endl;
            pageNotEmpty = SelectPaging(client, path, limit, lastCity, lastNumber);
        }
    }
    catch (const TYdbErrorException& e) {
        std::cerr << "Execution failed due to fatal error:" << std::endl;
        PrintStatus(e.Status);
        return false;
    }

    return true;
}
