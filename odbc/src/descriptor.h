#pragma once

#include "utils/error_manager.h"

#include <sql.h>
#include <sqlext.h>

#include <string>
#include <vector>

namespace NYdb {
namespace NOdbc {

class TStatement;

enum class EDescType {
    AppRow,
    AppParam,
    ImpRow,
    ImpParam,
    Explicit,
};

struct TExplicitDescRec {
    std::string Name;
    SQLSMALLINT Type = 0;
    SQLSMALLINT SubType = 0;
    SQLLEN Length = 0;
    SQLSMALLINT Precision = 0;
    SQLSMALLINT Scale = 0;
    SQLSMALLINT Nullable = SQL_NULLABLE;
    SQLPOINTER DataPtr = nullptr;
    SQLLEN Indicator = 0;
    SQLLEN OctetLength = 0;
};

class TDescriptor : public TErrorManager {
public:
    TDescriptor(EDescType type, TStatement* stmt = nullptr);

    EDescType GetDescType() const noexcept { return Type_; }
    TStatement* GetStatement() const noexcept { return Stmt_; }

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
    SQLSMALLINT GetRecordCount() const;
    bool GetExplicitRecord(SQLSMALLINT recNumber, TExplicitDescRec& out) const;
    TExplicitDescRec& GetOrCreateExplicitRecord(SQLSMALLINT recNumber);

    EDescType Type_;
    TStatement* Stmt_;
    std::vector<TExplicitDescRec> ExplicitRecs_;
};

} // namespace NOdbc
} // namespace NYdb
