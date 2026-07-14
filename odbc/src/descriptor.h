#pragma once

#include "utils/error_manager.h"

#include <sql.h>
#include <sqlext.h>

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
    bool AtExecComplete = false;
    SQLLEN AtExecIndicator = 0;
    std::string AtExecChunk;
};

class TDescriptor : public TErrorManager {
public:
    TDescriptor(EDescType type, TConnection* conn);
    ~TDescriptor();

    EDescType GetDescType() const noexcept { return Type_; }
    TConnection* GetConnection() const noexcept { return Conn_; }

    SQLULEN GetArraySize() const noexcept { return ArraySize_; }
    SQLULEN GetBindType() const noexcept { return BindType_; }
    SQLULEN* GetBindOffsetPtr() const noexcept { return BindOffsetPtr_; }
    SQLUSMALLINT* GetArrayStatusPtr() const noexcept { return ArrayStatusPtr_; }
    SQLULEN* GetRowsProcessedPtr() const noexcept { return RowsProcessedPtr_; }
    void SetArraySize(SQLULEN value) noexcept { ArraySize_ = value; }
    void SetBindType(SQLULEN value) noexcept { BindType_ = value; }
    void SetBindOffsetPtr(SQLULEN* value) noexcept { BindOffsetPtr_ = value; }
    void SetArrayStatusPtr(SQLUSMALLINT* value) noexcept { ArrayStatusPtr_ = value; }
    void SetRowsProcessedPtr(SQLULEN* value) noexcept { RowsProcessedPtr_ = value; }

    TDescRecord& Record(SQLSMALLINT number);
    const TDescRecord* FindRecord(SQLSMALLINT number) const noexcept;
    TDescRecord* FindRecord(SQLSMALLINT number) noexcept;
    void RemoveRecord(SQLSMALLINT number);
    void ClearRecords() noexcept { Records_.clear(); }
    SQLSMALLINT GetRecordCount() const noexcept;

    void Attach(TStatement* stmt);
    void Detach(TStatement* stmt);

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

    static TDescriptor* FromHandle(SQLHDESC handle);

private:
    EDescType Type_;
    TConnection* Conn_;
    SQLULEN ArraySize_ = 1;
    SQLULEN BindType_ = SQL_BIND_BY_COLUMN;
    SQLULEN* BindOffsetPtr_ = nullptr;
    SQLUSMALLINT* ArrayStatusPtr_ = nullptr;
    SQLULEN* RowsProcessedPtr_ = nullptr;
    std::vector<TDescRecord> Records_;
    std::vector<TStatement*> Statements_;
};

} // namespace NYdb::NOdbc
