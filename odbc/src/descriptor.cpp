#include "descriptor.h"

#include "statement.h"
#include "utils/diag.h"
#include "utils/sql_type_map.h"

#include <algorithm>
#include <cstring>

namespace NYdb::NOdbc {
namespace {

SQLULEN CTypeSize(SQLSMALLINT type, SQLLEN length) {
    switch (type) {
        case SQL_C_CHAR: case SQL_C_WCHAR: case SQL_C_BINARY:
            return static_cast<SQLULEN>(std::max<SQLLEN>(length, 0));
        case SQL_C_BIT: case SQL_C_TINYINT: case SQL_C_UTINYINT: return sizeof(SQLCHAR);
        case SQL_C_SHORT: case SQL_C_USHORT: return sizeof(SQLSMALLINT);
        case SQL_C_LONG: case SQL_C_ULONG: return sizeof(SQLINTEGER);
        case SQL_C_SBIGINT: case SQL_C_UBIGINT: return sizeof(SQLBIGINT);
        case SQL_C_FLOAT: return sizeof(SQLREAL);
        case SQL_C_DOUBLE: return sizeof(SQLDOUBLE);
        case SQL_C_TYPE_DATE: return sizeof(SQL_DATE_STRUCT);
#if defined(SQL_C_DATE) && SQL_C_DATE != SQL_C_TYPE_DATE
        case SQL_C_DATE: return sizeof(SQL_DATE_STRUCT);
#endif
        case SQL_C_TYPE_TIME: return sizeof(SQL_TIME_STRUCT);
#if defined(SQL_C_TIME) && SQL_C_TIME != SQL_C_TYPE_TIME
        case SQL_C_TIME: return sizeof(SQL_TIME_STRUCT);
#endif
        case SQL_C_TYPE_TIMESTAMP: return sizeof(SQL_TIMESTAMP_STRUCT);
#if defined(SQL_C_TIMESTAMP) && SQL_C_TIMESTAMP != SQL_C_TYPE_TIMESTAMP
        case SQL_C_TIMESTAMP: return sizeof(SQL_TIMESTAMP_STRUCT);
#endif
        case SQL_C_GUID: return sizeof(SQLGUID);
        default: return static_cast<SQLULEN>(std::max<SQLLEN>(length, 0));
    }
}

SQLPOINTER Offset(SQLPOINTER pointer, SQLULEN offset, SQLULEN index, SQLULEN stride) {
    return pointer
        ? static_cast<unsigned char*>(pointer) + offset + index * stride
        : nullptr;
}

bool IsCharacter(SQLSMALLINT type) {
    return type == SQL_CHAR || type == SQL_VARCHAR || type == SQL_LONGVARCHAR
        || type == SQL_WCHAR || type == SQL_WVARCHAR || type == SQL_WLONGVARCHAR;
}

SQLSMALLINT DateTimeCode(SQLSMALLINT type) {
    switch (type) {
        case SQL_TYPE_DATE: return SQL_CODE_DATE;
        case SQL_TYPE_TIME: return SQL_CODE_TIME;
        case SQL_TYPE_TIMESTAMP: return SQL_CODE_TIMESTAMP;
        default: return 0;
    }
}

std::string TypeName(SQLSMALLINT type) {
    const TSqlTypeSpec* spec = FindSqlTypeSpec(type);
    return spec ? std::string(spec->Name) : type == SQL_GUID ? "GUID" : "";
}

SQLRETURN WriteString(TErrorManager& errors, const std::string& text, SQLPOINTER value,
                      SQLINTEGER bufferLength, SQLINTEGER* lengthPtr) {
    if (lengthPtr) {
        *lengthPtr = static_cast<SQLINTEGER>(text.size());
    }
    if (!value) {
        return SQL_SUCCESS;
    }
    if (bufferLength < 0) {
        return Diag::AddInvalidBufferLength(errors);
    }
    if (bufferLength == 0) {
        return text.empty() ? SQL_SUCCESS : Diag::AddRightTruncated(errors);
    }
    const auto copyLength = std::min<size_t>(text.size(), static_cast<size_t>(bufferLength - 1));
    std::memcpy(value, text.data(), copyLength);
    static_cast<char*>(value)[copyLength] = '\0';
    return copyLength == text.size() ? SQL_SUCCESS : Diag::AddRightTruncated(errors);
}

template<typename T>
SQLRETURN WriteScalar(SQLPOINTER value, T scalar) {
    *static_cast<T*>(value) = scalar;
    return SQL_SUCCESS;
}

} // namespace

TDescriptor::TDescriptor(EDescType type, TConnection* conn)
    : Type_(type)
    , Conn_(conn) {
    if (Type_ == EDescType::Explicit) {
        Conn_->RegisterDescriptor(this);
    }
}

TDescriptor::~TDescriptor() {
    while (!Statements_.empty()) {
        Statements_.back()->DetachDescriptor(this);
    }
    if (Type_ == EDescType::Explicit) {
        Conn_->UnregisterDescriptor(this);
    }
}

TDescriptor* TDescriptor::FromHandle(SQLHDESC handle) {
    if (!handle) {
        throw TOdbcException("HY000", 0, "Invalid handle", SQL_INVALID_HANDLE);
    }
    return static_cast<TDescriptor*>(handle);
}

TDescRecord& TDescriptor::Record(SQLSMALLINT number) {
    if (number < 1) {
        throw TOdbcException("07009", 0, "Invalid descriptor index");
    }
    if (Records_.size() < static_cast<size_t>(number)) {
        Records_.resize(static_cast<size_t>(number));
    }
    TDescRecord& record = Records_[static_cast<size_t>(number - 1)];
    record.Active = true;
    return record;
}

const TDescRecord* TDescriptor::FindRecord(SQLSMALLINT number) const noexcept {
    if (number < 1 || static_cast<size_t>(number) > Records_.size()) {
        return nullptr;
    }
    const TDescRecord& record = Records_[static_cast<size_t>(number - 1)];
    return record.Active ? &record : nullptr;
}

TDescRecord* TDescriptor::FindRecord(SQLSMALLINT number) noexcept {
    return const_cast<TDescRecord*>(std::as_const(*this).FindRecord(number));
}

void TDescriptor::RemoveRecord(SQLSMALLINT number) {
    if (number > 0 && static_cast<size_t>(number) <= Records_.size()) {
        Records_[static_cast<size_t>(number - 1)] = {};
        while (!Records_.empty() && !Records_.back().Active) {
            Records_.pop_back();
        }
    }
}

SQLSMALLINT TDescriptor::GetRecordCount() const noexcept {
    return static_cast<SQLSMALLINT>(Records_.size());
}

TResolvedBinding TDescriptor::ResolveBinding(
    const TDescRecord& record, SQLULEN index) const noexcept {
    const SQLULEN offset = BindOffsetPtr_ ? *BindOffsetPtr_ : 0;
    const SQLULEN dataStride = BindType_ == SQL_BIND_BY_COLUMN
        ? CTypeSize(record.Type, record.OctetLength) : BindType_;
    const SQLULEN lengthStride = BindType_ == SQL_BIND_BY_COLUMN ? sizeof(SQLLEN) : BindType_;
    return {
        Offset(record.DataPtr, offset, index, dataStride),
        static_cast<SQLLEN*>(Offset(record.IndicatorPtr, offset, index, lengthStride)),
        static_cast<SQLLEN*>(Offset(record.OctetLengthPtr, offset, index, lengthStride)),
    };
}

void TDescriptor::Attach(TStatement* stmt) {
    if (Type_ == EDescType::Explicit
        && std::find(Statements_.begin(), Statements_.end(), stmt) == Statements_.end()) {
        Statements_.push_back(stmt);
    }
}

void TDescriptor::Detach(TStatement* stmt) {
    std::erase(Statements_, stmt);
}

SQLRETURN TDescriptor::GetDescField(SQLSMALLINT recNumber, SQLSMALLINT field, SQLPOINTER value,
                                    SQLINTEGER bufferLength, SQLINTEGER* lengthPtr) {
    const bool stringField = field == SQL_DESC_BASE_COLUMN_NAME || field == SQL_DESC_NAME
        || field == SQL_DESC_TYPE_NAME || field == SQL_DESC_LOCAL_TYPE_NAME
        || field == SQL_DESC_LITERAL_PREFIX || field == SQL_DESC_LITERAL_SUFFIX;
    if (!value && (!stringField || !lengthPtr)) {
        return Diag::AddNullPointer(*this);
    }
    switch (field) {
        case SQL_DESC_ALLOC_TYPE:
            return WriteScalar(value, static_cast<SQLSMALLINT>(
                Type_ == EDescType::Explicit ? SQL_DESC_ALLOC_USER : SQL_DESC_ALLOC_AUTO));
        case SQL_DESC_COUNT: return WriteScalar(value, GetRecordCount());
        case SQL_DESC_ARRAY_SIZE: return WriteScalar(value, ArraySize_);
        case SQL_DESC_BIND_TYPE: return WriteScalar(value, BindType_);
        case SQL_DESC_BIND_OFFSET_PTR: return WriteScalar(value, BindOffsetPtr_);
        case SQL_DESC_ARRAY_STATUS_PTR: return WriteScalar(value, ArrayStatusPtr_);
        case SQL_DESC_ROWS_PROCESSED_PTR: return WriteScalar(value, RowsProcessedPtr_);
        default: break;
    }

    const TDescRecord* record = FindRecord(recNumber);
    if (!record) {
        return recNumber > GetRecordCount()
            ? SQL_NO_DATA
            : AddError("07009", 0, "Invalid descriptor index");
    }
    switch (field) {
        case SQL_DESC_BASE_COLUMN_NAME:
        case SQL_DESC_NAME:
            return WriteString(*this, record->Name, value, bufferLength, lengthPtr);
        case SQL_DESC_TYPE_NAME:
        case SQL_DESC_LOCAL_TYPE_NAME:
            return WriteString(*this, TypeName(record->Type), value, bufferLength, lengthPtr);
        case SQL_DESC_LITERAL_PREFIX:
        case SQL_DESC_LITERAL_SUFFIX:
            return WriteString(
                *this, IsCharacter(record->Type) || DateTimeCode(record->Type) ? "'" : "",
                value, bufferLength, lengthPtr);
        case SQL_DESC_TYPE:
            return WriteScalar(value, static_cast<SQLSMALLINT>(
                DateTimeCode(record->Type) ? SQL_DATETIME : record->Type));
        case SQL_DESC_CONCISE_TYPE: return WriteScalar(value, record->Type);
        case SQL_DESC_DATETIME_INTERVAL_CODE:
            return WriteScalar(value, DateTimeCode(record->Type));
        case SQL_DESC_LENGTH: return WriteScalar(value, record->Length);
        case SQL_DESC_OCTET_LENGTH: return WriteScalar(value, record->OctetLength);
        case SQL_DESC_DISPLAY_SIZE: return WriteScalar(value, record->Length);
        case SQL_DESC_PRECISION: return WriteScalar(value, record->Precision);
        case SQL_DESC_SCALE: return WriteScalar(value, record->Scale);
        case SQL_DESC_NULLABLE: return WriteScalar(value, record->Nullable);
        case SQL_DESC_CASE_SENSITIVE:
            return WriteScalar(value, static_cast<SQLSMALLINT>(IsCharacter(record->Type)));
        case SQL_DESC_FIXED_PREC_SCALE:
            return WriteScalar(value, static_cast<SQLSMALLINT>(
                record->Type == SQL_DECIMAL || record->Type == SQL_NUMERIC));
        case SQL_DESC_SEARCHABLE:
            return WriteScalar(value, static_cast<SQLSMALLINT>(SQL_SEARCHABLE));
        case SQL_DESC_UNNAMED:
            return WriteScalar(value, static_cast<SQLSMALLINT>(
                record->Name.empty() ? SQL_UNNAMED : SQL_NAMED));
        case SQL_DESC_UNSIGNED: return WriteScalar(value, static_cast<SQLSMALLINT>(SQL_FALSE));
        case SQL_DESC_UPDATABLE:
            return WriteScalar(value, static_cast<SQLSMALLINT>(SQL_ATTR_READONLY));
        case SQL_DESC_PARAMETER_TYPE: return WriteScalar(value, record->ParameterType);
        case SQL_DESC_DATA_PTR: return WriteScalar(value, record->DataPtr);
        case SQL_DESC_INDICATOR_PTR: return WriteScalar(value, record->IndicatorPtr);
        case SQL_DESC_OCTET_LENGTH_PTR: return WriteScalar(value, record->OctetLengthPtr);
        default: return Diag::AddNotImplemented(*this);
    }
}

SQLRETURN TDescriptor::GetDescRec(SQLSMALLINT recNumber, SQLCHAR* name, SQLSMALLINT bufferLength,
                                  SQLSMALLINT* nameLengthPtr, SQLSMALLINT* typePtr,
                                  SQLSMALLINT* subTypePtr, SQLLEN* lengthPtr,
                                  SQLSMALLINT* precisionPtr, SQLSMALLINT* scalePtr,
                                  SQLSMALLINT* nullablePtr) {
    const TDescRecord* record = FindRecord(recNumber);
    if (!record) {
        return recNumber > GetRecordCount()
            ? SQL_NO_DATA
            : AddError("07009", 0, "Invalid descriptor index");
    }
    SQLINTEGER nameLength = 0;
    SQLRETURN result = SQL_SUCCESS;
    if (name) {
        result = WriteString(*this, record->Name, name, bufferLength, &nameLength);
    } else {
        nameLength = static_cast<SQLINTEGER>(record->Name.size());
    }
    if (nameLengthPtr) *nameLengthPtr = static_cast<SQLSMALLINT>(nameLength);
    if (typePtr) *typePtr = DateTimeCode(record->Type) ? SQL_DATETIME : record->Type;
    if (subTypePtr) *subTypePtr = DateTimeCode(record->Type);
    if (lengthPtr) *lengthPtr = record->Length;
    if (precisionPtr) *precisionPtr = record->Precision;
    if (scalePtr) *scalePtr = record->Scale;
    if (nullablePtr) *nullablePtr = record->Nullable;
    return result;
}

SQLRETURN TDescriptor::SetDescField(SQLSMALLINT recNumber, SQLSMALLINT field, SQLPOINTER value,
                                    SQLINTEGER bufferLength) {
    switch (field) {
        case SQL_DESC_COUNT: {
            const auto count = static_cast<SQLSMALLINT>(reinterpret_cast<intptr_t>(value));
            if (count < 0) return AddError("HY024", 0, "Invalid SQL_DESC_COUNT value");
            Records_.resize(static_cast<size_t>(count));
            for (auto& record : Records_) record.Active = true;
            return SQL_SUCCESS;
        }
        case SQL_DESC_ARRAY_SIZE: {
            const auto size = static_cast<SQLULEN>(reinterpret_cast<uintptr_t>(value));
            if (size == 0) return AddError("HY024", 0, "Invalid SQL_DESC_ARRAY_SIZE value");
            ArraySize_ = size;
            return SQL_SUCCESS;
        }
        case SQL_DESC_BIND_TYPE:
            BindType_ = static_cast<SQLULEN>(reinterpret_cast<uintptr_t>(value));
            return SQL_SUCCESS;
        case SQL_DESC_BIND_OFFSET_PTR: BindOffsetPtr_ = static_cast<SQLULEN*>(value); return SQL_SUCCESS;
        case SQL_DESC_ARRAY_STATUS_PTR: ArrayStatusPtr_ = static_cast<SQLUSMALLINT*>(value); return SQL_SUCCESS;
        case SQL_DESC_ROWS_PROCESSED_PTR: RowsProcessedPtr_ = static_cast<SQLULEN*>(value); return SQL_SUCCESS;
        default: break;
    }

    if (Type_ == EDescType::ImpRow) {
        return AddError("HY016", 0, "Cannot modify an implementation row descriptor");
    }

    TDescRecord& record = Record(recNumber);
    const auto integer = reinterpret_cast<intptr_t>(value);
    switch (field) {
        case SQL_DESC_TYPE:
        case SQL_DESC_CONCISE_TYPE: record.Type = static_cast<SQLSMALLINT>(integer); return SQL_SUCCESS;
        case SQL_DESC_LENGTH: record.Length = static_cast<SQLLEN>(integer); return SQL_SUCCESS;
        case SQL_DESC_OCTET_LENGTH: record.OctetLength = static_cast<SQLLEN>(integer); return SQL_SUCCESS;
        case SQL_DESC_PRECISION: record.Precision = static_cast<SQLSMALLINT>(integer); return SQL_SUCCESS;
        case SQL_DESC_SCALE: record.Scale = static_cast<SQLSMALLINT>(integer); return SQL_SUCCESS;
        case SQL_DESC_NULLABLE: record.Nullable = static_cast<SQLSMALLINT>(integer); return SQL_SUCCESS;
        case SQL_DESC_PARAMETER_TYPE: record.ParameterType = static_cast<SQLSMALLINT>(integer); return SQL_SUCCESS;
        case SQL_DESC_DATA_PTR: record.DataPtr = value; return SQL_SUCCESS;
        case SQL_DESC_INDICATOR_PTR: record.IndicatorPtr = static_cast<SQLLEN*>(value); return SQL_SUCCESS;
        case SQL_DESC_OCTET_LENGTH_PTR: record.OctetLengthPtr = static_cast<SQLLEN*>(value); return SQL_SUCCESS;
        case SQL_DESC_NAME:
            if (!value) return Diag::AddNullPointer(*this);
            record.Name = bufferLength == SQL_NTS
                ? std::string(static_cast<const char*>(value))
                : std::string(static_cast<const char*>(value), static_cast<size_t>(bufferLength));
            return SQL_SUCCESS;
        default: return Diag::AddNotImplemented(*this);
    }
}

SQLRETURN TDescriptor::SetDescRec(SQLSMALLINT recNumber, SQLSMALLINT type, SQLSMALLINT subType,
                                  SQLLEN length, SQLSMALLINT precision, SQLSMALLINT scale,
                                  SQLPOINTER dataPtr, SQLLEN* stringLengthPtr,
                                  SQLLEN* indicatorPtr) {
    if (Type_ == EDescType::ImpRow) {
        return AddError("HY016", 0, "Cannot modify an implementation row descriptor");
    }
    TDescRecord& record = Record(recNumber);
    record.Type = type;
    record.SubType = subType;
    record.Length = length;
    record.OctetLength = length;
    record.Precision = precision;
    record.Scale = scale;
    record.DataPtr = dataPtr;
    record.OctetLengthPtr = stringLengthPtr;
    record.IndicatorPtr = indicatorPtr;
    return SQL_SUCCESS;
}

SQLRETURN TDescriptor::CopyDesc(TDescriptor* target) {
    if (!target) return Diag::AddNullPointer(*this);
    if (target->Type_ == EDescType::ImpRow) {
        return AddError("HY016", 0, "Cannot modify an implementation row descriptor");
    }
    target->ArraySize_ = ArraySize_;
    target->BindType_ = BindType_;
    target->BindOffsetPtr_ = BindOffsetPtr_;
    target->ArrayStatusPtr_ = ArrayStatusPtr_;
    target->RowsProcessedPtr_ = RowsProcessedPtr_;
    target->Records_ = Records_;
    return SQL_SUCCESS;
}

} // namespace NYdb::NOdbc
