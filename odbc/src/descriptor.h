#pragma once

#include "utils/attr.h"
#include "connection.h"

#include "odbc_compat.h"

#include <string>
#include <vector>

namespace NYdb::NOdbc {

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

class TDescriptor : public THandle, private TDescriptorState {
    using THeaderProperties = TScalarProperties<
        TScalarProperty<SQL_DESC_ARRAY_SIZE, &THeader::ArraySize, true, false>,
        TScalarProperty<SQL_DESC_BIND_TYPE, &THeader::BindType>,
        TScalarProperty<SQL_DESC_BIND_OFFSET_PTR, &THeader::BindOffsetPtr>,
        TScalarProperty<SQL_DESC_ARRAY_STATUS_PTR, &THeader::ArrayStatusPtr>,
        TScalarProperty<SQL_DESC_ROWS_PROCESSED_PTR, &THeader::RowsProcessedPtr>>;

public:
    explicit TDescriptor(std::shared_ptr<TConnection> conn, EDescType type = EDescType::Explicit);

    EDescType GetDescType() const noexcept { return Type_; }
    TConnection& GetConnection() const noexcept { return static_cast<TConnection&>(*GetParent()); }

    // Statement access locks this descriptor; binding snapshots are statement-owned.
    void Clear();
    uint64_t GetGeneration() const { return Generation_.load(std::memory_order_relaxed); }
    void Snapshot(TDescriptorState& state, uint64_t& generation) const;
    void SnapshotHeader(TDescriptorState& state, uint64_t& generation) const;

    // ODBC entrypoints already hold the handle operation lock.
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
    static SQLRETURN Copy(SQLHDESC source, SQLHDESC target);

private:
    friend class TStatement;

    // Callbacks consume or mutate records within this scope; no references escape.
    template<class Fn>
    auto WithLock(Fn&& fn) {
        std::lock_guard lock(OperationMutex_);
        return std::forward<Fn>(fn)(*this);
    }

    template<class Fn>
    auto WithLock(TDescriptor& other, Fn&& fn) {
        std::scoped_lock lock(OperationMutex_, other.OperationMutex_);
        return std::forward<Fn>(fn)(*this, other);
    }

    TDescRecord& Record(SQLSMALLINT number);
    void RemoveRecord(SQLSMALLINT number);
    void ClearRecords() noexcept { Records_.clear(); Changed(); }
    void SnapshotUnlocked(TDescriptorState& state, uint64_t& generation) const;
    SQLRETURN CopyDesc(TDescriptor& target);
    void Changed(bool schema = true);

    EDescType Type_;
    std::atomic<uint64_t> Generation_ = 1;
};

} // namespace NYdb::NOdbc
