#include "statement.h"

#include "utils/sql_like.h"
#include "utils/sql_type_map.h"
#include "utils/types.h"
#include "utils/util.h"

#include <algorithm>
#include <cctype>
#include <limits>
#include <ranges>
#include <string_view>
#include <utility>

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
    N("NON_UNIQUE", SQL_SMALLINT),
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

using EEntry = NScheme::ESchemeEntryType;
constexpr std::pair<EEntry, std::string_view> kTableTypes[] = {
    {EEntry::Table, "TABLE"}, {EEntry::View, "VIEW"}, {EEntry::ColumnStore, "COLUMN_STORE"},
    {EEntry::ColumnTable, "COLUMN_TABLE"}, {EEntry::Sequence, "SEQUENCE"},
    {EEntry::Replication, "REPLICATION"}, {EEntry::Topic, "TOPIC"},
    {EEntry::ExternalTable, "EXTERNAL_TABLE"},
    {EEntry::ExternalDataSource, "EXTERNAL_DATA_SOURCE"}, {EEntry::ResourcePool, "RESOURCE_POOL"},
    {EEntry::PqGroup, "PQ_GROUP"}, {EEntry::RtmrVolume, "RTMR_VOLUME"},
    {EEntry::BlockStoreVolume, "BLOCK_STORE_VOLUME"},
    {EEntry::CoordinationNode, "COORDINATION_NODE"}, {EEntry::Unknown, "UNKNOWN"},
    {EEntry::SysView, "SYSTEM VIEW"}, {EEntry::Transfer, "TRANSFER"},
};

bool IsTable(EEntry type) {
    return type == EEntry::Table || type == EEntry::ColumnTable;
}

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

TOdbcScalar Int32OrNull(uint64_t value) {
    return value <= static_cast<uint64_t>(std::numeric_limits<SQLINTEGER>::max())
        ? I(static_cast<SQLINTEGER>(value))
        : Null();
}

bool IsExplicitEmpty(const TMetadataArgument& value) {
    return value && value->empty();
}

bool IsSpecialValue(const TMetadataArgument& value, std::string_view special) {
    return value && *value == special;
}

bool IsDateTimeType(SQLSMALLINT type) {
    return type == SQL_TYPE_DATE || type == SQL_TYPE_TIME || type == SQL_TYPE_TIMESTAMP;
}

TOdbcScalar GetSqlDataType(SQLSMALLINT type) {
    return I(IsDateTimeType(type) ? SQL_DATETIME : type);
}

TOdbcScalar GetDateTimeSub(SQLSMALLINT type) {
    switch (type) {
        case SQL_TYPE_DATE: return I(SQL_CODE_DATE);
        case SQL_TYPE_TIME: return I(SQL_CODE_TIME);
        case SQL_TYPE_TIMESTAMP: return I(SQL_CODE_TIMESTAMP);
        default: return Null();
    }
}

TOdbcScalar GetColumnSize(const TYdbTypeInfo& type) {
    return type.ColumnSize ? I(static_cast<SQLINTEGER>(type.ColumnSize)) : Null();
}

TOdbcScalar GetBufferLength(const TYdbTypeInfo& type) {
    switch (type.SqlType) {
        case SQL_BIT: case SQL_TINYINT: return I(sizeof(SQLCHAR));
        case SQL_SMALLINT: return I(sizeof(SQLSMALLINT));
        case SQL_INTEGER: case SQL_REAL: return I(sizeof(SQLINTEGER));
        case SQL_BIGINT: case SQL_DOUBLE: return I(sizeof(SQLBIGINT));
        case SQL_TYPE_DATE: return I(sizeof(SQL_DATE_STRUCT));
        case SQL_TYPE_TIME: return I(sizeof(SQL_TIME_STRUCT));
        case SQL_TYPE_TIMESTAMP: return I(sizeof(SQL_TIMESTAMP_STRUCT));
        case SQL_GUID: return I(sizeof(SQLGUID));
        default: return GetColumnSize(type);
    }
}

TOdbcScalar GetCharOctetLength(const TYdbTypeInfo& type) {
    switch (type.SqlType) {
        case SQL_CHAR: case SQL_VARCHAR: case SQL_LONGVARCHAR:
        case SQL_WCHAR: case SQL_WVARCHAR: case SQL_WLONGVARCHAR:
        case SQL_BINARY: case SQL_VARBINARY:
        case SQL_LONGVARBINARY:
            return GetColumnSize(type);
        default: return Null();
    }
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
void DescribeTable(TConnection* connection, const std::string& path,
                   Visitor&& visitor, bool withTableStatistics = false) {
    auto client = connection->GetTableClient();
    if (!client) {
        throw TOdbcException("HY000", 0, "No client connection");
    }
    auto status = client->RetryOperationSync(
        [path, withTableStatistics, &visitor](NTable::TSession session) -> TStatus {
            auto settings = NTable::TDescribeTableSettings()
                .WithTableStatistics(withTableStatistics);
            auto result = session.DescribeTable(path, settings).ExtractValueSync();
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

SQLRETURN TStatement::Columns(const TMetadataArgument& catalogName,
                              const TMetadataArgument& schemaName,
                              const TMetadataArgument& tableName,
                              const TMetadataArgument& columnName) {
    if (Attributes_.GetMetadataId() == SQL_TRUE
        && (!catalogName || !schemaName || !tableName || !columnName)) {
        return AddError("HY009", 0, "Identifier arguments must not be null");
    }
    ResetForMetadata();
    const bool patternsAllowed = Attributes_.GetMetadataId() != SQL_TRUE;
    if (!MetadataNamespaceMatches(catalogName, schemaName, false, patternsAllowed)) {
        SetCursor(CreateVirtualCursor(kColumnsSchema));
        return SQL_SUCCESS;
    }

    const std::string catalog = GetMetadataCatalogName(Conn_);
    TTable table;
    for (const auto& entry : GetMetadataEntries(tableName, patternsAllowed)) {
        if (!IsTable(entry.Type)) {
            continue;
        }
        DescribeTable(Conn_, entry.Name, [&](const auto& description) {
            const auto& columns = description.GetTableColumns();
            const auto& primaryKeyColumns = description.GetPrimaryKeyColumns();
            for (size_t index = 0; index < columns.size(); ++index) {
                const auto& column = columns[index];
                const bool matches = !columnName
                    || (patternsAllowed ? SqlLikeMatch(column.Name, *columnName)
                                        : column.Name == *columnName);
                if (!matches) {
                    continue;
                }
                const TYdbTypeInfo type = DescribeYdbType(column.Type);
                const TOdbcScalar size = GetColumnSize(type);
                const bool notNull = type.Nullable == SQL_NO_NULLS
                    || (column.NotNull && *column.NotNull)
                    || std::ranges::find(primaryKeyColumns, column.Name) != primaryKeyColumns.end();
                table.push_back({
                    catalog, Null(), GetMetadataTableName(entry.Name), column.Name, I(type.SqlType),
                    type.TypeName, size, GetBufferLength(type), Maybe(type.DecimalDigits),
                    Maybe(type.Radix), I(notNull ? SQL_NO_NULLS : SQL_NULLABLE), Null(), Null(),
                    GetSqlDataType(type.SqlType), GetDateTimeSub(type.SqlType),
                    GetCharOctetLength(type), I(static_cast<SQLINTEGER>(index + 1)),
                    std::string(notNull ? "NO" : "YES"),
                });
            }
        });
    }
    SetCursor(CreateVirtualCursor(kColumnsSchema, std::move(table)));
    return SQL_SUCCESS;
}

SQLRETURN TStatement::Tables(const TMetadataArgument& catalogName,
                             const TMetadataArgument& schemaName,
                             const TMetadataArgument& tableName,
                             const TMetadataArgument& tableType) {
    if (Attributes_.GetMetadataId() == SQL_TRUE
        && (!catalogName || !schemaName || !tableName)) {
        return AddError("HY009", 0, "Identifier arguments must not be null");
    }
    ResetForMetadata();

    const bool emptySchema = IsExplicitEmpty(schemaName);
    const bool emptyTable = IsExplicitEmpty(tableName);
    const bool emptyType = IsExplicitEmpty(tableType);
    if (IsSpecialValue(catalogName, SQL_ALL_CATALOGS)
        && emptySchema && emptyTable && emptyType) {
        SetCursor(CreateVirtualCursor(kTablesSchema, {{
            GetMetadataCatalogName(Conn_), Null(), Null(), Null(), Null(),
        }}));
        return SQL_SUCCESS;
    }
    if (IsSpecialValue(schemaName, SQL_ALL_SCHEMAS)
        && IsExplicitEmpty(catalogName) && emptyTable && emptyType) {
        SetCursor(CreateVirtualCursor(kTablesSchema));
        return SQL_SUCCESS;
    }
    if (IsSpecialValue(tableType, SQL_ALL_TABLE_TYPES)
        && IsExplicitEmpty(catalogName) && emptySchema && emptyTable) {
        std::vector<std::string_view> types;
        for (const auto& [_, type] : kTableTypes) {
            types.push_back(type);
        }
        std::ranges::sort(types);
        TTable table;
        for (const std::string_view type : types) {
            table.push_back({Null(), Null(), Null(), std::string(type), Null()});
        }
        SetCursor(CreateVirtualCursor(kTablesSchema, std::move(table)));
        return SQL_SUCCESS;
    }

    const bool patternsAllowed = Attributes_.GetMetadataId() != SQL_TRUE;
    if (!MetadataNamespaceMatches(catalogName, schemaName,
                                  patternsAllowed, patternsAllowed)) {
        SetCursor(CreateVirtualCursor(kTablesSchema));
        return SQL_SUCCESS;
    }

    const std::string catalog = GetMetadataCatalogName(Conn_);
    TTable table;
    auto entries = GetMetadataEntries(tableName, patternsAllowed);
    std::ranges::sort(entries, [this](const auto& lhs, const auto& rhs) {
        return std::pair{GetTableType(lhs.Type).value_or(""), GetMetadataTableName(lhs.Name)}
            < std::pair{GetTableType(rhs.Type).value_or(""), GetMetadataTableName(rhs.Name)};
    });
    const std::string_view typeFilter = tableType ? std::string_view(*tableType) : std::string_view{};
    for (const auto& entry : entries) {
        const auto type = GetTableType(entry.Type);
        if (type && MatchesTableTypeFilter(typeFilter, *type)) {
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

SQLRETURN TStatement::Statistics(const TMetadataArgument& catalogName,
                                 const TMetadataArgument& schemaName,
                                 const TMetadataArgument& tableName,
                                 SQLUSMALLINT unique, SQLUSMALLINT accuracy) {
    if (unique != SQL_INDEX_UNIQUE && unique != SQL_INDEX_ALL) {
        return AddError("HY100", 0, "Invalid index uniqueness option");
    }
    if (accuracy != SQL_QUICK && accuracy != SQL_ENSURE) {
        return AddError("HY101", 0, "Invalid statistics accuracy option");
    }
    if (!tableName) {
        return AddError("HY009", 0, "TableName must not be null");
    }
    if (Attributes_.GetMetadataId() == SQL_TRUE && (!catalogName || !schemaName)) {
        return AddError("HY009", 0, "Identifier arguments must not be null");
    }
    ResetForMetadata();
    if (!MetadataNamespaceMatches(catalogName, schemaName, false, false)
        || tableName->empty()) {
        SetCursor(CreateVirtualCursor(kStatisticsSchema));
        return SQL_SUCCESS;
    }

    auto entries = GetMetadataEntries(tableName, false);
    TTable table;
    if (!entries.empty() && IsTable(entries.front().Type)) {
        const auto& entry = entries.front();
        const std::string catalog = GetMetadataCatalogName(Conn_);
        const std::string tableNameResult = GetMetadataTableName(entry.Name);
        const bool withTableStatistics = accuracy == SQL_ENSURE;
        DescribeTable(Conn_, entry.Name, [&](const auto& description) {
            table.push_back({
                catalog, Null(), tableNameResult, Null(), Null(), Null(), I(SQL_TABLE_STAT),
                Null(), Null(), Null(),
                withTableStatistics ? Int32OrNull(description.GetTableRows()) : Null(),
                Null(), Null(),
            });

            auto indexes = description.GetIndexDescriptions();
            const auto indexOrder = [](const auto& index) {
                return std::pair{index.GetIndexType() != NTable::EIndexType::GlobalUnique,
                                 index.GetIndexName()};
            };
            std::ranges::sort(indexes, {}, indexOrder);
            for (const auto& index : indexes) {
                const bool isUnique = index.GetIndexType() == NTable::EIndexType::GlobalUnique;
                if (unique == SQL_INDEX_UNIQUE && !isUnique) {
                    continue;
                }
                SQLSMALLINT ordinal = 1;
                for (const auto& column : index.GetIndexColumns()) {
                    table.push_back({
                        catalog, Null(), tableNameResult, I(isUnique ? SQL_FALSE : SQL_TRUE),
                        Null(), index.GetIndexName(), I(SQL_INDEX_OTHER), I(ordinal++), column,
                        Null(), Null(), Null(), Null(),
                    });
                }
            }
        }, withTableStatistics);
    }
    SetCursor(CreateVirtualCursor(kStatisticsSchema, std::move(table)));
    return SQL_SUCCESS;
}

SQLRETURN TStatement::SpecialColumns(const std::string& catalogName, const std::string& schemaName,
                                     const std::string& tableName, SQLUSMALLINT identifierType,
                                     SQLUSMALLINT) {
    if (identifierType != SQL_BEST_ROWID) {
        return AddError("HYC00", 0, "Optional feature not implemented");
    }
    ResetForMetadata();
    const std::optional<std::string> catalog = catalogName.empty()
        ? std::nullopt : std::optional<std::string>{catalogName};
    const std::optional<std::string> schema = schemaName.empty()
        ? std::nullopt : std::optional<std::string>{schemaName};
    if (!MetadataNamespaceMatches(catalog, schema, false, false)) {
        SetCursor(CreateVirtualCursor(kSpecialColumnsSchema));
        return SQL_SUCCESS;
    }

    auto entries = GetMetadataEntries(std::optional<std::string>{tableName}, false);
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
                const TOdbcScalar size = GetColumnSize(type);
                table.push_back({I(SQL_SCOPE_SESSION), pkName, I(type.SqlType), type.TypeName,
                                 size, size, Maybe(type.DecimalDigits), I(SQL_PC_NOT_PSEUDO)});
            }
        });
    }
    SetCursor(CreateVirtualCursor(kSpecialColumnsSchema, std::move(table)));
    return SQL_SUCCESS;
}

SQLRETURN TStatement::PrimaryKeys(const TMetadataArgument& catalogName,
                                  const TMetadataArgument& schemaName,
                                  const TMetadataArgument& tableName) {
    if (!tableName) {
        return AddError("HY009", 0, "TableName must not be null");
    }
    if (Attributes_.GetMetadataId() == SQL_TRUE && (!catalogName || !schemaName)) {
        return AddError("HY009", 0, "Identifier arguments must not be null");
    }
    ResetForMetadata();
    if (!MetadataNamespaceMatches(catalogName, schemaName, false, false)
        || tableName->empty()) {
        SetCursor(CreateVirtualCursor(kPrimaryKeysSchema));
        return SQL_SUCCESS;
    }

    auto entries = GetMetadataEntries(tableName, false);
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

std::vector<NScheme::TSchemeEntry> TStatement::GetMetadataEntries(
    const TMetadataArgument& tableName, bool patternsAllowed) {
    std::vector<NScheme::TSchemeEntry> entries;
    if (tableName && tableName->empty()) {
        return entries;
    }

    const std::string catalog = Conn_->GetCatalogBinding().Catalog;
    const std::string prefix = catalog.empty() || catalog == "/" ? "/" : catalog + "/";
    const std::string qualifiedName = tableName
        ? (tableName->starts_with('/') ? *tableName : prefix + *tableName) : std::string{};
    if (tableName) {
        if (catalog != "/" && qualifiedName != catalog && !qualifiedName.starts_with(prefix)) {
            return entries;
        }
    }

    VisitEntry(catalog.empty() ? "/" : catalog, qualifiedName,
               tableName ? patternsAllowed : true, entries);
    std::ranges::sort(entries, {}, &NScheme::TSchemeEntry::Name);
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

bool TStatement::MetadataNamespaceMatches(
    const TMetadataArgument& catalog,
    const TMetadataArgument& schema,
    bool catalogPatternsAllowed,
    bool schemaPatternsAllowed) const {
    const auto matches = [](std::string_view value, std::string_view argument, bool patterns) {
        return patterns ? SqlLikeMatch(value, argument) : value == argument;
    };
    const bool schemaMatches = !schema || matches("", *schema, schemaPatternsAllowed);
    if (!catalog) {
        return schemaMatches;
    }
    std::string_view catalogArgument = *catalog;
    if (catalogArgument.starts_with('/')) {
        catalogArgument.remove_prefix(1);
    }
    return schemaMatches
        && matches(GetMetadataCatalogName(Conn_), catalogArgument, catalogPatternsAllowed);
}

void TStatement::VisitEntry(const std::string& path, const std::string& tableName,
                            bool patternsAllowed,
                            std::vector<NScheme::TSchemeEntry>& result) {
    auto client = Conn_->GetSchemeClient();
    if (!client) {
        throw TOdbcException("HY000", 0, "No client connection");
    }
    auto listing = client->ListDirectory(path).ExtractValueSync();
    NStatusHelpers::ThrowOnError(listing);
    for (const auto& entry : listing.GetChildren()) {
        const std::string fullPath = path == "/" ? path + entry.Name : path + "/" + entry.Name;
        if (entry.Type == NScheme::ESchemeEntryType::Directory
            || entry.Type == NScheme::ESchemeEntryType::SubDomain) {
            VisitEntry(fullPath, tableName, patternsAllowed, result);
        } else if (tableName.empty()
                   || (patternsAllowed ? SqlLikeMatch(fullPath, tableName)
                                       : fullPath == tableName)) {
            result.push_back(entry);
            result.back().Name = fullPath;
        }
    }
}

std::optional<std::string> TStatement::GetTableType(NScheme::ESchemeEntryType type) {
    for (const auto& [entryType, name] : kTableTypes) {
        if (entryType == type) {
            return std::string(name);
        }
    }
    return std::nullopt;
}

} // namespace NYdb::NOdbc
