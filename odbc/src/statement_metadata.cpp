#include "statement.h"

#include "utils/types.h"
#include "utils/sql_like.h"
#include "utils/type_info_rows.h"
#include "utils/cursor.h"
#include "utils/util.h"

#include <ydb-cpp-sdk/client/value/value.h>

#include <algorithm>
#include <cctype>
#include <optional>
#include <ranges>

namespace NYdb {
namespace NOdbc {

namespace {

bool MatchesTableTypeFilter(const std::string& filter, const std::string& entryType) {
    if (filter.empty()) {
        return true;
    }
    size_t start = 0;
    while (start <= filter.size()) {
        const size_t comma = filter.find(',', start);
        std::string token = filter.substr(start, comma == std::string::npos ? std::string::npos : comma - start);
        while (!token.empty() && std::isspace(static_cast<unsigned char>(token.front()))) {
            token.erase(token.begin());
        }
        while (!token.empty() && std::isspace(static_cast<unsigned char>(token.back()))) {
            token.pop_back();
        }
        if (token.size() >= 2 && token.front() == '\'' && token.back() == '\'') {
            token = token.substr(1, token.size() - 2);
        }
        if (!token.empty() && token.size() == entryType.size() &&
            StartsWithPrefix(entryType.c_str(), entryType.size(), token.c_str(), token.size())) {
            return true;
        }
        if (comma == std::string::npos) {
            break;
        }
        start = comma + 1;
    }
    return false;
}

namespace NColumnsRow {
constexpr int kTableCat = 0;
constexpr int kTableSchem = 1;
constexpr int kTableName = 2;
constexpr int kColumnName = 3;
constexpr int kDataType = 4;
constexpr int kTypeName = 5;
constexpr int kColumnSize = 6;
constexpr int kBufferLength = 7;
constexpr int kDecimalDigits = 8;
constexpr int kNumPrecRadix = 9;
constexpr int kNullable = 10;
constexpr int kRemarks = 11;
constexpr int kColumnDef = 12;
constexpr int kSqlDataType = 13;
constexpr int kSqlDatetimeSub = 14;
constexpr int kCharOctetLength = 15;
constexpr int kOrdinalPosition = 16;
constexpr int kIsNullable = 17;
} // namespace NColumnsRow

} // namespace

SQLRETURN TStatement::Columns(const std::string& catalogName,
                              const std::string& schemaName,
                              const std::string& tableName,
                              const std::string& columnName) {
    ResetForMetadata();

    std::vector<TColumnMeta> columns = {
        {"TABLE_CAT", SQL_VARCHAR, 128, SQL_NULLABLE},
        {"TABLE_SCHEM", SQL_VARCHAR, 128, SQL_NULLABLE},
        {"TABLE_NAME", SQL_VARCHAR, 128, SQL_NO_NULLS},
        {"COLUMN_NAME", SQL_VARCHAR, 128, SQL_NO_NULLS},
        {"DATA_TYPE", SQL_INTEGER, 0, SQL_NO_NULLS},
        {"TYPE_NAME", SQL_VARCHAR, 128, SQL_NO_NULLS},
        {"COLUMN_SIZE", SQL_INTEGER, 0, SQL_NULLABLE},
        {"BUFFER_LENGTH", SQL_INTEGER, 0, SQL_NULLABLE},
        {"DECIMAL_DIGITS", SQL_INTEGER, 0, SQL_NULLABLE},
        {"NUM_PREC_RADIX", SQL_INTEGER, 0, SQL_NULLABLE},
        {"NULLABLE", SQL_INTEGER, 0, SQL_NO_NULLS},
        {"REMARKS", SQL_VARCHAR, 762, SQL_NULLABLE},
        {"COLUMN_DEF", SQL_VARCHAR, 254, SQL_NULLABLE},
        {"SQL_DATA_TYPE", SQL_INTEGER, 0, SQL_NO_NULLS},
        {"SQL_DATETIME_SUB", SQL_INTEGER, 0, SQL_NULLABLE},
        {"CHAR_OCTET_LENGTH", SQL_INTEGER, 0, SQL_NULLABLE},
        {"ORDINAL_POSITION", SQL_INTEGER, 0, SQL_NO_NULLS},
        {"IS_NULLABLE", SQL_VARCHAR, 254, SQL_NO_NULLS}
    };

    auto entries = GetPatternEntries(tableName);

    TTable table;
    table.reserve(entries.size());

    if (entries.empty()) {
        Cursor_ = CreateVirtualCursor(this, columns, table);
        return SQL_SUCCESS;
    }

    for (const auto& entry : entries) {
        if (entry.Type != NScheme::ESchemeEntryType::Table &&
            entry.Type != NScheme::ESchemeEntryType::ColumnTable) {
            continue;
        }

        auto tableClient = Conn_->GetTableClient();
        if (!tableClient) {
            throw TOdbcException("HY000", 0, "No client connection");
        }

        auto status = tableClient->RetryOperationSync([this, path = entry.Name, &table, &columnName](NTable::TSession session) -> TStatus {
            auto result = session.DescribeTable(path).ExtractValueSync();
            NStatusHelpers::ThrowOnError(result);

            auto columns = result.GetTableDescription().GetTableColumns();

            auto columnMatches = [&](const NTable::TTableColumn& column) {
                if (columnName.empty()) {
                    return true;
                }
                if (Attributes_.GetMetadataId() == SQL_TRUE) {
                    return column.Name == columnName;
                }
                return SqlLikeMatch(column.Name, columnName);
            };

            bool foundColumn = false;
            for (size_t columnIndex = 0; columnIndex < columns.size(); ++columnIndex) {
                const auto& column = columns[columnIndex];
                if (!columnMatches(column)) {
                    continue;
                }
                foundColumn = true;

                const auto sqlType = GetTypeId(column.Type);
                const auto colSize = GetColumnSize(sqlType);
                const auto decDigits = GetDecimalDigits(column.Type);
                const auto radix = GetRadix(column.Type);
                const std::optional<SQLINTEGER> colSizeOpt = colSize > 0 ? std::optional<SQLINTEGER>(static_cast<SQLINTEGER>(colSize)) : std::nullopt;

                table.push_back({
                    TValueBuilder().OptionalUtf8(std::nullopt).Build(),
                    TValueBuilder().OptionalUtf8(std::nullopt).Build(),
                    TValueBuilder().Utf8(path).Build(),
                    TValueBuilder().Utf8(column.Name).Build(),
                    TValueBuilder().Int16(sqlType).Build(),
                    TValueBuilder().Utf8(column.Type.ToString()).Build(),
                    TValueBuilder().OptionalInt32(colSizeOpt).Build(),
                    TValueBuilder().OptionalInt32(colSizeOpt).Build(),
                    TValueBuilder().OptionalInt16(decDigits).Build(),
                    TValueBuilder().OptionalInt16(radix).Build(),
                    TValueBuilder().Int16(column.NotNull && *column.NotNull ? SQL_NO_NULLS : SQL_NULLABLE).Build(),
                    TValueBuilder().OptionalUtf8(std::nullopt).Build(),
                    TValueBuilder().OptionalUtf8(std::nullopt).Build(),
                    TValueBuilder().Int16(sqlType).Build(),
                    TValueBuilder().OptionalInt16(std::nullopt).Build(),
                    TValueBuilder().OptionalInt32(colSizeOpt).Build(),
                    TValueBuilder().OptionalInt32(columnIndex + 1).Build(),
                    TValueBuilder().Utf8(column.NotNull && *column.NotNull ? "NO" : "YES").Build(),
                });
            }
            if (!foundColumn && !columnName.empty()) {
                return TStatus(EStatus::SUCCESS, {});
            }
            return TStatus(EStatus::SUCCESS, {});
        });

        NStatusHelpers::ThrowOnError(status);
    }

    Cursor_ = CreateVirtualCursor(this, columns, table);
    return SQL_SUCCESS;
}

SQLRETURN TStatement::Tables(const std::string& catalogName,
                             const std::string& schemaName,
                             const std::string& tableName,
                             const std::string& tableType) {
    ResetForMetadata();

    std::vector<TColumnMeta> columns = {
        {"TABLE_CAT", SQL_VARCHAR, 128, SQL_NULLABLE},
        {"TABLE_SCHEM", SQL_VARCHAR, 128, SQL_NULLABLE},
        {"TABLE_NAME", SQL_VARCHAR, 128, SQL_NO_NULLS},
        {"TABLE_TYPE", SQL_VARCHAR, 128, SQL_NO_NULLS},
        {"REMARKS", SQL_VARCHAR, 254, SQL_NULLABLE}
    };

    auto entries = GetPatternEntries(tableName);

    TTable table;
    table.reserve(entries.size());

    for (const auto& entry : entries) {
        const auto entryType = GetTableType(entry.Type);
        if (!entryType || !MatchesTableTypeFilter(tableType, *entryType)) {
            continue;
        }

        table.push_back({
            TValueBuilder().OptionalUtf8(std::nullopt).Build(),
            TValueBuilder().OptionalUtf8(std::nullopt).Build(),
            TValueBuilder().Utf8(entry.Name).Build(),
            TValueBuilder().Utf8(*entryType).Build(),
            TValueBuilder().OptionalUtf8(std::nullopt).Build(),
        });
    }

    Cursor_ = CreateVirtualCursor(this, columns, table);
    return SQL_SUCCESS;
}

SQLRETURN TStatement::GetTypeInfo(SQLSMALLINT dataType) {
    ResetForMetadata();

    static const std::vector<TColumnMeta> columns = {
        {"TYPE_NAME", SQL_VARCHAR, 128, SQL_NO_NULLS},
        {"DATA_TYPE", SQL_SMALLINT, 0, SQL_NO_NULLS},
        {"COLUMN_SIZE", SQL_INTEGER, 0, SQL_NULLABLE},
        {"LITERAL_PREFIX", SQL_VARCHAR, 128, SQL_NULLABLE},
        {"LITERAL_SUFFIX", SQL_VARCHAR, 128, SQL_NULLABLE},
        {"CREATE_PARAMS", SQL_VARCHAR, 128, SQL_NULLABLE},
        {"NULLABLE", SQL_SMALLINT, 0, SQL_NO_NULLS},
        {"CASE_SENSITIVE", SQL_SMALLINT, 0, SQL_NO_NULLS},
        {"SEARCHABLE", SQL_SMALLINT, 0, SQL_NO_NULLS},
        {"UNSIGNED_ATTRIBUTE", SQL_CHAR, 1, SQL_NULLABLE},
        {"FIXED_PREC_SCALE", SQL_SMALLINT, 0, SQL_NO_NULLS},
        {"AUTO_UNIQUE_VALUE", SQL_SMALLINT, 0, SQL_NO_NULLS},
        {"LOCAL_TYPE_NAME", SQL_VARCHAR, 128, SQL_NULLABLE},
        {"MINIMUM_SCALE", SQL_SMALLINT, 0, SQL_NULLABLE},
        {"MAXIMUM_SCALE", SQL_SMALLINT, 0, SQL_NULLABLE},
        {"SQL_DATA_TYPE", SQL_SMALLINT, 0, SQL_NO_NULLS},
        {"SQL_DATETIME_SUB", SQL_SMALLINT, 0, SQL_NULLABLE},
        {"NUM_PREC_RADIX", SQL_INTEGER, 0, SQL_NULLABLE},
        {"INTERVAL_PRECISION", SQL_SMALLINT, 0, SQL_NULLABLE},
    };

    Cursor_ = CreateVirtualCursor(this, columns, BuildTypeInfoRows(dataType));
    return SQL_SUCCESS;
}

SQLRETURN TStatement::Statistics(const std::string& /*catalogName*/,
                                 const std::string& /*schemaName*/,
                                 const std::string& /*tableName*/,
                                 SQLUSMALLINT /*unique*/,
                                 SQLUSMALLINT /*accuracy*/) {
    ResetForMetadata();

    static const std::vector<TColumnMeta> columns = {
        {"TABLE_CAT", SQL_VARCHAR, 128, SQL_NULLABLE},
        {"TABLE_SCHEM", SQL_VARCHAR, 128, SQL_NULLABLE},
        {"TABLE_NAME", SQL_VARCHAR, 128, SQL_NO_NULLS},
        {"NON_UNIQUE", SQL_CHAR, 1, SQL_NO_NULLS},
        {"INDEX_QUALIFIER", SQL_VARCHAR, 128, SQL_NULLABLE},
        {"INDEX_NAME", SQL_VARCHAR, 128, SQL_NULLABLE},
        {"TYPE", SQL_SMALLINT, 0, SQL_NO_NULLS},
        {"ORDINAL_POSITION", SQL_SMALLINT, 0, SQL_NULLABLE},
        {"COLUMN_NAME", SQL_VARCHAR, 128, SQL_NULLABLE},
        {"ASC_OR_DESC", SQL_CHAR, 1, SQL_NULLABLE},
        {"CARDINALITY", SQL_INTEGER, 0, SQL_NULLABLE},
        {"PAGES", SQL_INTEGER, 0, SQL_NULLABLE},
        {"FILTER_CONDITION", SQL_VARCHAR, 128, SQL_NULLABLE},
    };

    Cursor_ = CreateVirtualCursor(this, columns, TTable{});
    return SQL_SUCCESS;
}

SQLRETURN TStatement::SpecialColumns(const std::string& /*catalogName*/,
                                     const std::string& /*schemaName*/,
                                     const std::string& tableName,
                                     SQLUSMALLINT identifierType,
                                     SQLUSMALLINT /*scope*/) {
    if (identifierType != SQL_BEST_ROWID) {
        return AddError("HYC00", 0, "Optional feature not implemented");
    }

    ResetForMetadata();

    std::vector<TColumnMeta> columns = {
        {"SCOPE", SQL_SMALLINT, 0, SQL_NULLABLE},
        {"COLUMN_NAME", SQL_VARCHAR, 128, SQL_NO_NULLS},
        {"DATA_TYPE", SQL_SMALLINT, 0, SQL_NO_NULLS},
        {"TYPE_NAME", SQL_VARCHAR, 128, SQL_NO_NULLS},
        {"COLUMN_SIZE", SQL_INTEGER, 0, SQL_NULLABLE},
        {"BUFFER_LENGTH", SQL_INTEGER, 0, SQL_NULLABLE},
        {"DECIMAL_DIGITS", SQL_SMALLINT, 0, SQL_NULLABLE},
        {"PSEUDO_COLUMN", SQL_SMALLINT, 0, SQL_NO_NULLS},
    };

    TTable table;
    auto entries = GetPatternEntries(tableName);
    if (entries.size() != 1) {
        if (entries.empty()) {
            Cursor_ = CreateVirtualCursor(this, columns, table);
            return SQL_SUCCESS;
        }
        throw TOdbcException("HY000", 0, "Ambiguous table name");
    }

    auto tableClient = Conn_->GetTableClient();
    if (!tableClient) {
        throw TOdbcException("HY000", 0, "No client connection");
    }

    const std::string path = entries.front().Name;
    auto status = tableClient->RetryOperationSync([path, &table, &columns](NTable::TSession session) -> TStatus {
        auto result = session.DescribeTable(path).ExtractValueSync();
        NStatusHelpers::ThrowOnError(result);

        const auto& pkColumns = result.GetTableDescription().GetPrimaryKeyColumns();
        const auto& tableColumns = result.GetTableDescription().GetTableColumns();
        for (const auto& pkName : pkColumns) {
            const auto columnIt = std::ranges::find_if(tableColumns,
                [&](const NTable::TTableColumn& column) { return column.Name == pkName; });
            if (columnIt == tableColumns.end()) {
                continue;
            }
            const auto sqlType = GetTypeId(columnIt->Type);
            const auto colSize = GetColumnSize(sqlType);
            const std::optional<SQLINTEGER> colSizeOpt = colSize > 0 ? std::optional<SQLINTEGER>(static_cast<SQLINTEGER>(colSize)) : std::nullopt;
            table.push_back({
                TValueBuilder().OptionalInt16(SQL_SCOPE_SESSION).Build(),
                TValueBuilder().Utf8(pkName).Build(),
                TValueBuilder().Int16(sqlType).Build(),
                TValueBuilder().Utf8(columnIt->Type.ToString()).Build(),
                TValueBuilder().OptionalInt32(colSizeOpt).Build(),
                TValueBuilder().OptionalInt32(colSizeOpt).Build(),
                TValueBuilder().OptionalInt16(GetDecimalDigits(columnIt->Type)).Build(),
                TValueBuilder().Int16(SQL_PC_NOT_PSEUDO).Build(),
            });
        }
        return TStatus(EStatus::SUCCESS, {});
    });
    NStatusHelpers::ThrowOnError(status);

    Cursor_ = CreateVirtualCursor(this, columns, table);
    return SQL_SUCCESS;
}

SQLRETURN TStatement::PrimaryKeys(const std::string& /*catalogName*/,
                                  const std::string& /*schemaName*/,
                                  const std::string& tableName) {
    ResetForMetadata();

    std::vector<TColumnMeta> columns = {
        {"TABLE_CAT", SQL_VARCHAR, 128, SQL_NULLABLE},
        {"TABLE_SCHEM", SQL_VARCHAR, 128, SQL_NULLABLE},
        {"TABLE_NAME", SQL_VARCHAR, 128, SQL_NO_NULLS},
        {"COLUMN_NAME", SQL_VARCHAR, 128, SQL_NO_NULLS},
        {"KEY_SEQ", SQL_SMALLINT, 0, SQL_NO_NULLS},
        {"PK_NAME", SQL_VARCHAR, 128, SQL_NULLABLE},
    };

    TTable table;
    auto entries = GetPatternEntries(tableName);
    if (entries.size() != 1) {
        if (entries.empty()) {
            Cursor_ = CreateVirtualCursor(this, columns, table);
            return SQL_SUCCESS;
        }
        throw TOdbcException("HY000", 0, "Ambiguous table name");
    }

    auto tableClient = Conn_->GetTableClient();
    if (!tableClient) {
        throw TOdbcException("HY000", 0, "No client connection");
    }

    const std::string path = entries.front().Name;
    auto status = tableClient->RetryOperationSync([path, &table](NTable::TSession session) -> TStatus {
        auto result = session.DescribeTable(path).ExtractValueSync();
        NStatusHelpers::ThrowOnError(result);

        const auto& pkColumns = result.GetTableDescription().GetPrimaryKeyColumns();
        SQLSMALLINT keySeq = 1;
        for (const auto& pkName : pkColumns) {
            table.push_back({
                TValueBuilder().OptionalUtf8(std::nullopt).Build(),
                TValueBuilder().OptionalUtf8(std::nullopt).Build(),
                TValueBuilder().Utf8(path).Build(),
                TValueBuilder().Utf8(pkName).Build(),
                TValueBuilder().Int16(keySeq++).Build(),
                TValueBuilder().OptionalUtf8(std::nullopt).Build(),
            });
        }
        return TStatus(EStatus::SUCCESS, {});
    });
    NStatusHelpers::ThrowOnError(status);

    Cursor_ = CreateVirtualCursor(this, columns, table);
    return SQL_SUCCESS;
}

SQLRETURN TStatement::ForeignKeys(const std::string& /*pkCatalogName*/,
                                    const std::string& /*pkSchemaName*/,
                                    const std::string& /*pkTableName*/,
                                    const std::string& /*fkCatalogName*/,
                                    const std::string& /*fkSchemaName*/,
                                    const std::string& /*fkTableName*/) {
    ResetForMetadata();

    std::vector<TColumnMeta> columns = {
        {"PKTABLE_CAT", SQL_VARCHAR, 128, SQL_NULLABLE},
        {"PKTABLE_SCHEM", SQL_VARCHAR, 128, SQL_NULLABLE},
        {"PKTABLE_NAME", SQL_VARCHAR, 128, SQL_NO_NULLS},
        {"PKCOLUMN_NAME", SQL_VARCHAR, 128, SQL_NO_NULLS},
        {"FKTABLE_CAT", SQL_VARCHAR, 128, SQL_NULLABLE},
        {"FKTABLE_SCHEM", SQL_VARCHAR, 128, SQL_NULLABLE},
        {"FKTABLE_NAME", SQL_VARCHAR, 128, SQL_NO_NULLS},
        {"FKCOLUMN_NAME", SQL_VARCHAR, 128, SQL_NO_NULLS},
        {"KEY_SEQ", SQL_SMALLINT, 0, SQL_NO_NULLS},
        {"UPDATE_RULE", SQL_SMALLINT, 0, SQL_NULLABLE},
        {"DELETE_RULE", SQL_SMALLINT, 0, SQL_NULLABLE},
        {"FK_NAME", SQL_VARCHAR, 128, SQL_NULLABLE},
        {"PK_NAME", SQL_VARCHAR, 128, SQL_NULLABLE},
        {"DEFERRABILITY", SQL_SMALLINT, 0, SQL_NULLABLE},
    };

    Cursor_ = CreateVirtualCursor(this, columns, TTable{});
    return SQL_SUCCESS;
}

std::string TStatement::GetTraversalRoot(const std::string& pattern) const {
    if (pattern.empty()) {
        return "";
    }
    const auto hasWildcard = [](const std::string& value) {
        return value.find('%') != std::string::npos || value.find('_') != std::string::npos;
    };
    if (Attributes_.GetMetadataId() == SQL_TRUE && !hasWildcard(pattern)) {
        const auto pos = pattern.find_last_of('/');
        return pos == std::string::npos ? "" : pattern.substr(0, pos);
    }
    size_t wildPos = pattern.size();
    const auto pct = pattern.find('%');
    const auto usc = pattern.find('_');
    if (pct != std::string::npos) {
        wildPos = std::min(wildPos, pct);
    }
    if (usc != std::string::npos) {
        wildPos = std::min(wildPos, usc);
    }
    const std::string prefix = pattern.substr(0, wildPos);
    const auto pos = prefix.find_last_of('/');
    return pos == std::string::npos ? "" : prefix.substr(0, pos);
}

std::vector<NScheme::TSchemeEntry> TStatement::GetPatternEntries(const std::string& pattern) {
    std::vector<NScheme::TSchemeEntry> entries;
    VisitEntry(GetTraversalRoot(pattern), pattern, entries);
    return entries;
}

SQLRETURN TStatement::VisitEntry(const std::string& path, const std::string& pattern, std::vector<NScheme::TSchemeEntry>& resultEntries) {
    auto schemeClient = Conn_->GetSchemeClient();
    if (!schemeClient) {
        throw TOdbcException("HY000", 0, "No client connection");
    }
    auto listDirectoryResult = schemeClient->ListDirectory(path + "/").ExtractValueSync();
    NStatusHelpers::ThrowOnError(listDirectoryResult);

    for (const auto& entry : listDirectoryResult.GetChildren()) {
        std::string fullPath = path + "/" + entry.Name;
        if (entry.Type == NScheme::ESchemeEntryType::Directory ||
            entry.Type == NScheme::ESchemeEntryType::SubDomain) {
            VisitEntry(fullPath, pattern, resultEntries);
        } else if (IsPatternMatch(fullPath, pattern)) {
            NScheme::TSchemeEntry entryCopy = entry;
            entryCopy.Name = fullPath;
            resultEntries.push_back(entryCopy);
        }
    }
    return SQL_SUCCESS;
}

bool TStatement::IsPatternMatch(const std::string& path, const std::string& pattern) {
    if (pattern.empty()) {
        return true;
    }
    if (Attributes_.GetMetadataId() == SQL_TRUE) {
        return path == pattern;
    }
    return SqlLikeMatch(path, pattern);
}

std::optional<std::string> TStatement::GetTableType(NScheme::ESchemeEntryType type) {
    switch (type) {
        case NScheme::ESchemeEntryType::Table:
            return "TABLE";
        case NScheme::ESchemeEntryType::View:
            return "VIEW";
        case NScheme::ESchemeEntryType::ColumnStore:
            return "COLUMN_STORE";
        case NScheme::ESchemeEntryType::ColumnTable:
            return "COLUMN_TABLE";
        case NScheme::ESchemeEntryType::Sequence:
            return "SEQUENCE";
        case NScheme::ESchemeEntryType::Replication:
            return "REPLICATION";
        case NScheme::ESchemeEntryType::Topic:
            return "TOPIC";
        case NScheme::ESchemeEntryType::ExternalTable:
            return "EXTERNAL_TABLE";
        case NScheme::ESchemeEntryType::ExternalDataSource:
            return "EXTERNAL_DATA_SOURCE";
        case NScheme::ESchemeEntryType::ResourcePool:
            return "RESOURCE_POOL";
        case NScheme::ESchemeEntryType::PqGroup:
            return "PQ_GROUP";
        case NScheme::ESchemeEntryType::RtmrVolume:
            return "RTMR_VOLUME";
        case NScheme::ESchemeEntryType::BlockStoreVolume:
            return "BLOCK_STORE_VOLUME";
        case NScheme::ESchemeEntryType::CoordinationNode:
            return "COORDINATION_NODE";
        case NScheme::ESchemeEntryType::Unknown:
            return "UNKNOWN";
        case NScheme::ESchemeEntryType::SysView:
            return "SYSTEM VIEW";
        case NScheme::ESchemeEntryType::Transfer:
            return "TRANSFER";
        case NScheme::ESchemeEntryType::Directory:
        case NScheme::ESchemeEntryType::SubDomain:
            return std::nullopt;
        default:
            return std::nullopt;
    }
}

} // namespace NOdbc
} // namespace NYdb
