#include "statement.h"

#include "utils/sql_like.h"
#include "utils/sql_type_map.h"
#include "utils/types.h"
#include "utils/util.h"

#include <algorithm>
#include <cctype>
#include <ranges>
#include <string_view>

namespace NYdb::NOdbc {
namespace {

TColumnMeta C(std::string_view name, SQLSMALLINT type, SQLULEN size,
              SQLSMALLINT nullable = SQL_NULLABLE) {
    return {std::string(name), type, size, nullable};
}

TColumnMeta V(std::string_view name, SQLULEN size = 128,
              SQLSMALLINT nullable = SQL_NULLABLE) {
    return C(name, SQL_VARCHAR, size, nullable);
}

TColumnMeta N(std::string_view name, SQLSMALLINT type,
              SQLSMALLINT nullable = SQL_NULLABLE) {
    return C(name, type, 0, nullable);
}

const TColumnMeta kColumnsSchema[] = {
    V("TABLE_CAT"),
    V("TABLE_SCHEM"),
    V("TABLE_NAME", 128, SQL_NO_NULLS),
    V("COLUMN_NAME", 128, SQL_NO_NULLS),
    N("DATA_TYPE", SQL_SMALLINT, SQL_NO_NULLS),
    V("TYPE_NAME", 128, SQL_NO_NULLS),
    N("COLUMN_SIZE", SQL_INTEGER),
    N("BUFFER_LENGTH", SQL_INTEGER),
    N("DECIMAL_DIGITS", SQL_SMALLINT),
    N("NUM_PREC_RADIX", SQL_SMALLINT),
    N("NULLABLE", SQL_SMALLINT, SQL_NO_NULLS),
    V("REMARKS", 762),
    V("COLUMN_DEF", 254),
    N("SQL_DATA_TYPE", SQL_SMALLINT, SQL_NO_NULLS),
    N("SQL_DATETIME_SUB", SQL_SMALLINT),
    N("CHAR_OCTET_LENGTH", SQL_INTEGER),
    N("ORDINAL_POSITION", SQL_INTEGER, SQL_NO_NULLS),
    V("IS_NULLABLE", 254, SQL_NO_NULLS),
};

const TColumnMeta kTablesSchema[] = {
    V("TABLE_CAT"),
    V("TABLE_SCHEM"),
    V("TABLE_NAME", 128, SQL_NO_NULLS),
    V("TABLE_TYPE", 128, SQL_NO_NULLS),
    V("REMARKS", 254),
};

const TColumnMeta kTypeInfoSchema[] = {
    V("TYPE_NAME", 128, SQL_NO_NULLS),
    N("DATA_TYPE", SQL_SMALLINT, SQL_NO_NULLS),
    N("COLUMN_SIZE", SQL_INTEGER),
    V("LITERAL_PREFIX"),
    V("LITERAL_SUFFIX"),
    V("CREATE_PARAMS"),
    N("NULLABLE", SQL_SMALLINT, SQL_NO_NULLS),
    N("CASE_SENSITIVE", SQL_SMALLINT, SQL_NO_NULLS),
    N("SEARCHABLE", SQL_SMALLINT, SQL_NO_NULLS),
    N("UNSIGNED_ATTRIBUTE", SQL_SMALLINT),
    N("FIXED_PREC_SCALE", SQL_SMALLINT, SQL_NO_NULLS),
    N("AUTO_UNIQUE_VALUE", SQL_SMALLINT),
    V("LOCAL_TYPE_NAME"),
    N("MINIMUM_SCALE", SQL_SMALLINT),
    N("MAXIMUM_SCALE", SQL_SMALLINT),
    N("SQL_DATA_TYPE", SQL_SMALLINT, SQL_NO_NULLS),
    N("SQL_DATETIME_SUB", SQL_SMALLINT),
    N("NUM_PREC_RADIX", SQL_INTEGER),
    N("INTERVAL_PRECISION", SQL_SMALLINT),
};

const TColumnMeta kStatisticsSchema[] = {
    V("TABLE_CAT"),
    V("TABLE_SCHEM"),
    V("TABLE_NAME", 128, SQL_NO_NULLS),
    N("NON_UNIQUE", SQL_SMALLINT, SQL_NO_NULLS),
    V("INDEX_QUALIFIER"),
    V("INDEX_NAME"),
    N("TYPE", SQL_SMALLINT, SQL_NO_NULLS),
    N("ORDINAL_POSITION", SQL_SMALLINT),
    V("COLUMN_NAME"),
    C("ASC_OR_DESC", SQL_CHAR, 1),
    N("CARDINALITY", SQL_INTEGER),
    N("PAGES", SQL_INTEGER),
    V("FILTER_CONDITION"),
};

const TColumnMeta kSpecialColumnsSchema[] = {
    N("SCOPE", SQL_SMALLINT),
    V("COLUMN_NAME", 128, SQL_NO_NULLS),
    N("DATA_TYPE", SQL_SMALLINT, SQL_NO_NULLS),
    V("TYPE_NAME", 128, SQL_NO_NULLS),
    N("COLUMN_SIZE", SQL_INTEGER),
    N("BUFFER_LENGTH", SQL_INTEGER),
    N("DECIMAL_DIGITS", SQL_SMALLINT),
    N("PSEUDO_COLUMN", SQL_SMALLINT, SQL_NO_NULLS),
};

const TColumnMeta kPrimaryKeysSchema[] = {
    V("TABLE_CAT"),
    V("TABLE_SCHEM"),
    V("TABLE_NAME", 128, SQL_NO_NULLS),
    V("COLUMN_NAME", 128, SQL_NO_NULLS),
    N("KEY_SEQ", SQL_SMALLINT, SQL_NO_NULLS),
    V("PK_NAME"),
};

const TColumnMeta kForeignKeysSchema[] = {
    V("PKTABLE_CAT"),
    V("PKTABLE_SCHEM"),
    V("PKTABLE_NAME", 128, SQL_NO_NULLS),
    V("PKCOLUMN_NAME", 128, SQL_NO_NULLS),
    V("FKTABLE_CAT"),
    V("FKTABLE_SCHEM"),
    V("FKTABLE_NAME", 128, SQL_NO_NULLS),
    V("FKCOLUMN_NAME", 128, SQL_NO_NULLS),
    N("KEY_SEQ", SQL_SMALLINT, SQL_NO_NULLS),
    N("UPDATE_RULE", SQL_SMALLINT),
    N("DELETE_RULE", SQL_SMALLINT),
    V("FK_NAME"),
    V("PK_NAME"),
    N("DEFERRABILITY", SQL_SMALLINT),
};

const TColumnMeta kColumnPrivilegesSchema[] = {
    V("TABLE_CAT"),
    V("TABLE_SCHEM"),
    V("TABLE_NAME", 128, SQL_NO_NULLS),
    V("COLUMN_NAME", 128, SQL_NO_NULLS),
    V("GRANTOR"),
    V("GRANTEE", 128, SQL_NO_NULLS),
    V("PRIVILEGE", 128, SQL_NO_NULLS),
    V("IS_GRANTABLE"),
};

TOdbcScalar Null() {
    return std::monostate{};
}

template <class T>
TOdbcScalar I(T value) {
    return static_cast<int64_t>(value);
}

template <class T>
TOdbcScalar Maybe(const std::optional<T>& value) {
    return value ? I(*value) : Null();
}

std::string GetMetadataCatalogName(TConnection* connection) {
    std::string catalog = connection->GetCatalogBinding().Catalog;
    // TABLE_CAT is an identifier. The leading slash belongs to YDB's absolute
    // path syntax and is supplied separately as SQL_CATALOG_NAME_SEPARATOR.
    if (catalog.starts_with('/')) {
        catalog.erase(0, 1);
    }
    return catalog;
}

template <class Visitor>
void DescribeTable(TConnection* connection, const std::string& path, Visitor&& visitor) {
    auto client = connection->GetTableClient();
    if (!client) {
        throw TOdbcException("HY000", 0, "No client connection");
    }
    auto status = client->RetryOperationSync(
        [path, &visitor](NTable::TSession session) -> TStatus {
            auto result = session.DescribeTable(path).ExtractValueSync();
            NStatusHelpers::ThrowOnError(result);
            visitor(result.GetTableDescription());
            return TStatus(EStatus::SUCCESS, {});
        });
    NStatusHelpers::ThrowOnError(status);
}

bool MatchesTableTypeFilter(std::string_view filter, std::string_view entryType) {
    if (filter.empty()) {
        return true;
    }
    while (true) {
        const size_t comma = filter.find(',');
        std::string_view token = filter.substr(0, comma);
        while (!token.empty() && std::isspace(static_cast<unsigned char>(token.front()))) {
            token.remove_prefix(1);
        }
        while (!token.empty() && std::isspace(static_cast<unsigned char>(token.back()))) {
            token.remove_suffix(1);
        }
        if (token.size() >= 2 && token.front() == '\'' && token.back() == '\'') {
            token = token.substr(1, token.size() - 2);
        }
        if (token.size() == entryType.size()
            && StartsWithPrefix(entryType.data(), entryType.size(), token.data(), token.size())) {
            return true;
        }
        if (comma == std::string_view::npos) {
            return false;
        }
        filter.remove_prefix(comma + 1);
    }
}

TTable BuildTypeInfoRows(SQLSMALLINT dataType) {
    if (dataType == SQL_DATE) {
        dataType = SQL_TYPE_DATE;
    } else if (dataType == SQL_TIME) {
        dataType = SQL_TYPE_TIME;
    } else if (dataType == SQL_TIMESTAMP) {
        dataType = SQL_TYPE_TIMESTAMP;
    }

    TTable table;
    for (const TSqlTypeSpec& spec : GetSqlTypeSpecs()) {
        if ((dataType == SQL_ALL_TYPES && !spec.Advertise)
            || (dataType != SQL_ALL_TYPES && spec.Type != dataType)) {
            continue;
        }
        const std::string typeName(spec.YqlType);
        table.push_back({
            typeName, I(spec.Type), I(static_cast<SQLINTEGER>(spec.ColumnSize)), Null(), Null(),
            Null(), I(SQL_NULLABLE),
            I(SQL_FALSE), I(SQL_PRED_SEARCHABLE), Null(), I(SQL_FALSE), I(SQL_FALSE), typeName,
            I(0), I(0), I(spec.Type), I(0), I(10), I(0),
        });
    }
    return table;
}

} // namespace

SQLRETURN TStatement::Columns(const std::string& catalogName, const std::string& schemaName,
                              const std::string& tableName, const std::string& columnName) {
    ResetForMetadata();
    if (!MetadataNamespaceMatches(catalogName, schemaName)) {
        SetCursor(CreateVirtualCursor(kColumnsSchema));
        return SQL_SUCCESS;
    }

    const std::string catalog = GetMetadataCatalogName(Conn_);
    TTable table;
    for (const auto& entry : GetPatternEntries(tableName)) {
        if (entry.Type != NScheme::ESchemeEntryType::Table
            && entry.Type != NScheme::ESchemeEntryType::ColumnTable) {
            continue;
        }
        DescribeTable(Conn_, entry.Name, [&](const auto& description) {
            const auto& columns = description.GetTableColumns();
            const auto& primaryKeyColumns = description.GetPrimaryKeyColumns();
            for (size_t index = 0; index < columns.size(); ++index) {
                const auto& column = columns[index];
                const bool matches = columnName.empty()
                    || (Attributes_.GetMetadataId() == SQL_TRUE ? column.Name == columnName
                                                                : SqlLikeMatch(column.Name, columnName));
                if (!matches) {
                    continue;
                }
                const TYdbTypeInfo type = DescribeYdbType(column.Type);
                const TOdbcScalar size = type.ColumnSize
                    ? I(static_cast<SQLINTEGER>(type.ColumnSize)) : Null();
                const bool notNull = type.Nullable == SQL_NO_NULLS
                    || (column.NotNull && *column.NotNull)
                    || std::ranges::find(primaryKeyColumns, column.Name) != primaryKeyColumns.end();
                table.push_back({
                    catalog, Null(), GetMetadataTableName(entry.Name), column.Name, I(type.SqlType),
                    type.TypeName, size, size, Maybe(type.DecimalDigits), Maybe(type.Radix),
                    I(notNull ? SQL_NO_NULLS : SQL_NULLABLE), Null(), Null(), I(type.SqlType), Null(),
                    size, I(static_cast<SQLINTEGER>(index + 1)), std::string(notNull ? "NO" : "YES"),
                });
            }
        });
    }
    SetCursor(CreateVirtualCursor(kColumnsSchema, std::move(table)));
    return SQL_SUCCESS;
}

SQLRETURN TStatement::Tables(const std::string& catalogName, const std::string& schemaName,
                             const std::string& tableName, const std::string& tableType) {
    ResetForMetadata();
    if (!MetadataNamespaceMatches(catalogName, schemaName)) {
        SetCursor(CreateVirtualCursor(kTablesSchema));
        return SQL_SUCCESS;
    }

    const std::string catalog = GetMetadataCatalogName(Conn_);
    TTable table;
    for (const auto& entry : GetPatternEntries(tableName)) {
        const auto type = GetTableType(entry.Type);
        if (type && MatchesTableTypeFilter(tableType, *type)) {
            table.push_back({catalog, Null(), GetMetadataTableName(entry.Name), *type, Null()});
        }
    }
    SetCursor(CreateVirtualCursor(kTablesSchema, std::move(table)));
    return SQL_SUCCESS;
}

SQLRETURN TStatement::GetTypeInfo(SQLSMALLINT dataType) {
    ResetForMetadata();
    SetCursor(CreateVirtualCursor(kTypeInfoSchema, BuildTypeInfoRows(dataType)));
    return SQL_SUCCESS;
}

SQLRETURN TStatement::Statistics(const std::string&, const std::string&, const std::string&,
                                 SQLUSMALLINT, SQLUSMALLINT) {
    ResetForMetadata();
    SetCursor(CreateVirtualCursor(kStatisticsSchema));
    return SQL_SUCCESS;
}

SQLRETURN TStatement::SpecialColumns(const std::string& catalogName, const std::string& schemaName,
                                     const std::string& tableName, SQLUSMALLINT identifierType,
                                     SQLUSMALLINT) {
    if (identifierType != SQL_BEST_ROWID) {
        return AddError("HYC00", 0, "Optional feature not implemented");
    }
    ResetForMetadata();
    if (!MetadataNamespaceMatches(catalogName, schemaName)) {
        SetCursor(CreateVirtualCursor(kSpecialColumnsSchema));
        return SQL_SUCCESS;
    }

    auto entries = GetPatternEntries(tableName);
    if (entries.size() > 1) {
        throw TOdbcException("HY000", 0, "Ambiguous table name");
    }
    TTable table;
    if (!entries.empty()) {
        DescribeTable(Conn_, entries.front().Name, [&](const auto& description) {
            const auto& columns = description.GetTableColumns();
            for (const auto& pkName : description.GetPrimaryKeyColumns()) {
                const auto column = std::ranges::find_if(
                    columns, [&](const auto& item) { return item.Name == pkName; });
                if (column == columns.end()) {
                    continue;
                }
                const TYdbTypeInfo type = DescribeYdbType(column->Type);
                const TOdbcScalar size = type.ColumnSize
                    ? I(static_cast<SQLINTEGER>(type.ColumnSize)) : Null();
                table.push_back({I(SQL_SCOPE_SESSION), pkName, I(type.SqlType), type.TypeName,
                                 size, size, Maybe(type.DecimalDigits), I(SQL_PC_NOT_PSEUDO)});
            }
        });
    }
    SetCursor(CreateVirtualCursor(kSpecialColumnsSchema, std::move(table)));
    return SQL_SUCCESS;
}

SQLRETURN TStatement::PrimaryKeys(const std::string& catalogName, const std::string& schemaName,
                                  const std::string& tableName) {
    ResetForMetadata();
    if (!MetadataNamespaceMatches(catalogName, schemaName)) {
        SetCursor(CreateVirtualCursor(kPrimaryKeysSchema));
        return SQL_SUCCESS;
    }

    auto entries = GetPatternEntries(tableName);
    if (entries.size() > 1) {
        throw TOdbcException("HY000", 0, "Ambiguous table name");
    }
    TTable table;
    if (!entries.empty()) {
        const std::string catalog = GetMetadataCatalogName(Conn_);
        DescribeTable(Conn_, entries.front().Name, [&](const auto& description) {
            SQLSMALLINT sequence = 1;
            for (const auto& name : description.GetPrimaryKeyColumns()) {
                table.push_back({catalog, Null(), GetMetadataTableName(entries.front().Name),
                                 name, I(sequence++), Null()});
            }
        });
    }
    SetCursor(CreateVirtualCursor(kPrimaryKeysSchema, std::move(table)));
    return SQL_SUCCESS;
}

SQLRETURN TStatement::ForeignKeys(const std::string&, const std::string&, const std::string&,
                                  const std::string&, const std::string&, const std::string&) {
    ResetForMetadata();
    SetCursor(CreateVirtualCursor(kForeignKeysSchema));
    return SQL_SUCCESS;
}

SQLRETURN TStatement::ColumnPrivileges(const std::string&, const std::string&,
                                       const std::string&, const std::string&) {
    ResetForMetadata();
    SetCursor(CreateVirtualCursor(kColumnPrivilegesSchema));
    return SQL_SUCCESS;
}

std::string TStatement::GetTraversalRoot(const std::string& pattern) const {
    const size_t slash = pattern.rfind('/', pattern.find_first_of("%_"));
    return slash == std::string::npos ? "" : pattern.substr(0, slash);
}

std::vector<NScheme::TSchemeEntry> TStatement::GetPatternEntries(const std::string& pattern) {
    const std::string catalog = Conn_->GetCatalogBinding().Catalog;
    std::string searchPattern = pattern;
    if (!pattern.empty() && pattern.front() != '/' && pattern.find('/') == std::string::npos) {
        searchPattern = catalog + (catalog.empty() || catalog.back() != '/' ? "/" : "") + pattern;
    }
    std::vector<NScheme::TSchemeEntry> entries;
    VisitEntry(pattern.empty() ? catalog : GetTraversalRoot(searchPattern), searchPattern, entries);
    return entries;
}

std::string TStatement::GetMetadataTableName(const std::string& path) const {
    const std::string catalog = Conn_->GetCatalogBinding().Catalog;
    if (catalog == "/" && path.starts_with('/')) {
        return path.substr(1);
    }
    const std::string prefix = catalog + "/";
    return path.starts_with(prefix) ? path.substr(prefix.size()) : path;
}

bool TStatement::MetadataNamespaceMatches(const std::string& catalog, const std::string& schema) const {
    const auto matches = [&](const std::string& value, const std::string& pattern) {
        return pattern.empty() || (Attributes_.GetMetadataId() == SQL_TRUE
            ? value == pattern : SqlLikeMatch(value, pattern));
    };
    std::string normalizedCatalog = catalog;
    if (!normalizedCatalog.empty() && normalizedCatalog.front() != '/') {
        normalizedCatalog.insert(normalizedCatalog.begin(), '/');
    }
    return matches(Conn_->GetCatalogBinding().Catalog, normalizedCatalog) && matches("", schema);
}

SQLRETURN TStatement::VisitEntry(const std::string& path, const std::string& pattern,
                                 std::vector<NScheme::TSchemeEntry>& result) {
    auto client = Conn_->GetSchemeClient();
    if (!client) {
        throw TOdbcException("HY000", 0, "No client connection");
    }
    auto listing = client->ListDirectory(path + "/").ExtractValueSync();
    NStatusHelpers::ThrowOnError(listing);
    for (const auto& entry : listing.GetChildren()) {
        const std::string fullPath = path + "/" + entry.Name;
        if (entry.Type == NScheme::ESchemeEntryType::Directory
            || entry.Type == NScheme::ESchemeEntryType::SubDomain) {
            VisitEntry(fullPath, pattern, result);
        } else if (IsPatternMatch(fullPath, pattern)) {
            result.push_back(entry);
            result.back().Name = fullPath;
        }
    }
    return SQL_SUCCESS;
}

bool TStatement::IsPatternMatch(const std::string& path, const std::string& pattern) {
    return pattern.empty() || (Attributes_.GetMetadataId() == SQL_TRUE
        ? path == pattern : SqlLikeMatch(path, pattern));
}

std::optional<std::string> TStatement::GetTableType(NScheme::ESchemeEntryType type) {
    using E = NScheme::ESchemeEntryType;
    static constexpr std::pair<E, std::string_view> types[] = {
        {E::Table, "TABLE"}, {E::View, "VIEW"}, {E::ColumnStore, "COLUMN_STORE"},
        {E::ColumnTable, "COLUMN_TABLE"}, {E::Sequence, "SEQUENCE"},
        {E::Replication, "REPLICATION"}, {E::Topic, "TOPIC"},
        {E::ExternalTable, "EXTERNAL_TABLE"}, {E::ExternalDataSource, "EXTERNAL_DATA_SOURCE"},
        {E::ResourcePool, "RESOURCE_POOL"}, {E::PqGroup, "PQ_GROUP"},
        {E::RtmrVolume, "RTMR_VOLUME"}, {E::BlockStoreVolume, "BLOCK_STORE_VOLUME"},
        {E::CoordinationNode, "COORDINATION_NODE"}, {E::Unknown, "UNKNOWN"},
        {E::SysView, "SYSTEM VIEW"}, {E::Transfer, "TRANSFER"},
    };
    for (const auto& [entryType, name] : types) {
        if (entryType == type) {
            return std::string(name);
        }
    }
    return std::nullopt;
}

} // namespace NYdb::NOdbc
