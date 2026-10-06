#pragma once

#include "utils/attr.h"

#include "odbc_compat.h"

#include <string>
#include <vector>

namespace NYdb::NOdbc {

class TConnection;
class TStatement;

enum class EDescType {
    AppRow,
    AppParam,
    ImpRow,
    ImpParam,
    Explicit,
};

struct TDescRecord {
    std::string Name;
    SQLSMALLINT Type = SQL_UNKNOWN_TYPE;
    SQLSMALLINT SubType = 0;
    SQLLEN Length = 0;
    SQLLEN OctetLength = 0;
    SQLSMALLINT Precision = 0;
    SQLSMALLINT Scale = 0;
    SQLSMALLINT Nullable = SQL_NULLABLE;
    SQLPOINTER DataPtr = nullptr;
    SQLLEN* IndicatorPtr = nullptr;
    SQLLEN* OctetLengthPtr = nullptr;
    SQLSMALLINT ParameterType = SQL_PARAM_INPUT;
    bool Active = false;
    bool AtExec = false;
};

struct TResolvedBinding {
    SQLPOINTER Data = nullptr;
    SQLLEN* Indicator = nullptr;
    SQLLEN* OctetLength = nullptr;
};

class TDescriptorState {
    friend class TDescriptor;
protected:
    struct THeader {
        SQLULEN ArraySize = 1;
        SQLULEN BindType = SQL_BIND_BY_COLUMN;
        SQLULEN* BindOffsetPtr = nullptr;
        SQLUSMALLINT* ArrayStatusPtr = nullptr;
        SQLULEN* RowsProcessedPtr = nullptr;
    };

    THeader Header_;
    std::vector<TDescRecord> Records_;
    uint64_t SchemaGeneration_ = 0;

public:
    SQLULEN GetArraySize() const noexcept { return Header_.ArraySize; }
    SQLUSMALLINT* GetArrayStatusPtr() const noexcept { return Header_.ArrayStatusPtr; }
    SQLULEN* GetRowsProcessedPtr() const noexcept { return Header_.RowsProcessedPtr; }
    uint64_t GetSchemaGeneration() const noexcept { return SchemaGeneration_; }
    const TDescRecord* FindRecord(SQLSMALLINT number) const noexcept;
    SQLSMALLINT GetRecordCount() const noexcept;
    TResolvedBinding ResolveBinding(const TDescRecord& record, SQLULEN index) const noexcept;
};

class TDescriptor : public TErrorManager, public TDescriptorState {
    using THeaderProperties = TScalarProperties<
        TScalarProperty<SQL_DESC_ARRAY_SIZE, &THeader::ArraySize, true, false>,
        TScalarProperty<SQL_DESC_BIND_TYPE, &THeader::BindType>,
        TScalarProperty<SQL_DESC_BIND_OFFSET_PTR, &THeader::BindOffsetPtr>,
        TScalarProperty<SQL_DESC_ARRAY_STATUS_PTR, &THeader::ArrayStatusPtr>,
        TScalarProperty<SQL_DESC_ROWS_PROCESSED_PTR, &THeader::RowsProcessedPtr>>;

public:
    TDescriptor(EDescType type, TConnection* conn);
    ~TDescriptor();

    EDescType GetDescType() const noexcept { return Type_; }
    TConnection* GetConnection() const noexcept { return Conn_; }

    TDescRecord& Record(SQLSMALLINT number);
    using TDescriptorState::FindRecord;
    TDescRecord* FindRecord(SQLSMALLINT number) noexcept;
    void RemoveRecord(SQLSMALLINT number);
    void ClearRecords() noexcept { Records_.clear(); Changed(); }
    uint64_t GetGeneration() const { return Generation_.load(std::memory_order_relaxed); }
    void Snapshot(TDescriptorState& state, uint64_t& generation) const;
    void SnapshotHeader(TDescriptorState& state, uint64_t& generation) const;

    SQLRETURN GetDescField(SQLSMALLINT recNumber, SQLSMALLINT fieldIdentifier, SQLPOINTER value,
                           SQLINTEGER bufferLength, SQLINTEGER* stringLengthPtr);
    SQLRETURN GetDescRec(SQLSMALLINT recNumber, SQLCHAR* name, SQLSMALLINT bufferLength,
                         SQLSMALLINT* stringLengthPtr, SQLSMALLINT* typePtr, SQLSMALLINT* subTypePtr,
                         SQLLEN* lengthPtr, SQLSMALLINT* precisionPtr, SQLSMALLINT* scalePtr,
                         SQLSMALLINT* nullablePtr);
    SQLRETURN SetDescField(SQLSMALLINT recNumber, SQLSMALLINT fieldIdentifier, SQLPOINTER value,
                           SQLINTEGER bufferLength);
    SQLRETURN SetDescRec(SQLSMALLINT recNumber, SQLSMALLINT type, SQLSMALLINT subType, SQLLEN length,
                         SQLSMALLINT precision, SQLSMALLINT scale, SQLPOINTER dataPtr,
                         SQLLEN* stringLengthPtr, SQLLEN* indicatorPtr);
    SQLRETURN CopyDesc(TDescriptor* target);

private:
    void Changed(bool schema = true);

    EDescType Type_;
    TConnection* Conn_;
    std::atomic<uint64_t> Generation_ = 1;
};

} // namespace NYdb::NOdbc
