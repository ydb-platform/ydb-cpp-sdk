#include "descriptor.h"

#include "connection.h"
#include "utils/diag.h"
#include "utils/sql_type_map.h"

#include <algorithm>

namespace NYdb::NOdbc {
namespace {

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

template<typename T>
SQLRETURN WriteScalar(SQLPOINTER value, T scalar) {
    *static_cast<T*>(value) = scalar;
    return SQL_SUCCESS;
}

template<typename T, typename U>
void WriteIf(T* output, U value) {
    if (output) {
        *output = static_cast<T>(value);
    }
}

using TRecordProperties = TScalarProperties<
    TScalarProperty<SQL_DESC_TYPE, &TDescRecord::Type, false, true>,
    TScalarProperty<SQL_DESC_CONCISE_TYPE, &TDescRecord::Type>,
    TScalarProperty<SQL_DESC_LENGTH, &TDescRecord::Length>,
    TScalarProperty<SQL_DESC_OCTET_LENGTH, &TDescRecord::OctetLength>,
    TScalarProperty<SQL_DESC_DISPLAY_SIZE, &TDescRecord::Length, true, false>,
    TScalarProperty<SQL_DESC_PRECISION, &TDescRecord::Precision>,
    TScalarProperty<SQL_DESC_SCALE, &TDescRecord::Scale>,
    TScalarProperty<SQL_DESC_NULLABLE, &TDescRecord::Nullable>,
    TScalarProperty<SQL_DESC_PARAMETER_TYPE, &TDescRecord::ParameterType>,
    TScalarProperty<SQL_DESC_DATA_PTR, &TDescRecord::DataPtr>,
    TScalarProperty<SQL_DESC_INDICATOR_PTR, &TDescRecord::IndicatorPtr>,
    TScalarProperty<SQL_DESC_OCTET_LENGTH_PTR, &TDescRecord::OctetLengthPtr>>;

} // namespace

TDescriptor::TDescriptor(EDescType type, TConnection* conn)
    : Type_(type)
    , Conn_(conn) {
    SetLifecycle(conn->GetLifecycle());
}

void TDescriptor::Snapshot(TDescriptorState& state, uint64_t& generation) const {
    std::lock_guard lock(OperationMutex_);
    SnapshotUnlocked(state, generation);
}

void TDescriptor::SnapshotUnlocked(TDescriptorState& state, uint64_t& generation) const {
    state = static_cast<const TDescriptorState&>(*this);
    generation = GetGeneration();
}

void TDescriptor::SnapshotHeader(TDescriptorState& state, uint64_t& generation) const {
    std::lock_guard lock(OperationMutex_);
    // The implementation row descriptor is consumed only for status outputs.
    state.Header_ = Header_;
    generation = GetGeneration();
}

void TDescriptor::Clear() {
    std::lock_guard lock(OperationMutex_);
    ClearRecords();
}

TDescRecord& TDescriptor::Record(SQLSMALLINT number) {
    if (number < 1) {
        throw TOdbcException("07009", 0, "Invalid descriptor index");
    }
    if (Records_.size() < static_cast<size_t>(number)) {
        Records_.resize(static_cast<size_t>(number));
    }
    Changed();
    TDescRecord& record = Records_[static_cast<size_t>(number - 1)];
    record.Active = true;
    return record;
}

const TDescRecord* TDescriptorState::FindRecord(SQLSMALLINT number) const noexcept {
    if (number < 1 || static_cast<size_t>(number) > Records_.size()) {
        return nullptr;
    }
    const TDescRecord& record = Records_[static_cast<size_t>(number - 1)];
    return record.Active ? &record : nullptr;
}

void TDescriptor::RemoveRecord(SQLSMALLINT number) {
    if (number > 0 && static_cast<size_t>(number) <= Records_.size()) {
        Records_[static_cast<size_t>(number - 1)] = {};
        while (!Records_.empty() && !Records_.back().Active) {
            Records_.pop_back();
        }
        Changed();
    }
}

SQLSMALLINT TDescriptorState::GetRecordCount() const noexcept {
    return static_cast<SQLSMALLINT>(Records_.size());
}

TResolvedBinding TDescriptorState::ResolveBinding(
    const TDescRecord& record, SQLULEN index) const noexcept {
    const SQLULEN offset = Header_.BindOffsetPtr ? *Header_.BindOffsetPtr : 0;
    const SQLULEN dataStride = Header_.BindType == SQL_BIND_BY_COLUMN
        ? GetCTypeSize(record.Type, record.OctetLength) : Header_.BindType;
    const SQLULEN lengthStride = Header_.BindType == SQL_BIND_BY_COLUMN
        ? sizeof(SQLLEN) : Header_.BindType;
    return {
        Offset(record.DataPtr, offset, index, dataStride),
        static_cast<SQLLEN*>(Offset(record.IndicatorPtr, offset, index, lengthStride)),
        static_cast<SQLLEN*>(Offset(record.OctetLengthPtr, offset, index, lengthStride)),
    };
}

void TDescriptor::Changed(bool schema) {
    if (schema) {
        ++SchemaGeneration_;
    }
    Generation_.fetch_add(1, std::memory_order_relaxed);
}

SQLRETURN TDescriptor::GetDescField(SQLSMALLINT recNumber, SQLSMALLINT field, SQLPOINTER value,
                                    SQLINTEGER bufferLength, SQLINTEGER* lengthPtr) {
    const bool stringField = field == SQL_DESC_BASE_COLUMN_NAME || field == SQL_DESC_NAME
        || field == SQL_DESC_TYPE_NAME || field == SQL_DESC_LOCAL_TYPE_NAME
        || field == SQL_DESC_LITERAL_PREFIX || field == SQL_DESC_LITERAL_SUFFIX;
    if (!value && (!stringField || !lengthPtr)) {
        return Diag::AddNullPointer(*this);
    }
    if (field == SQL_DESC_ALLOC_TYPE) {
        return WriteScalar(value, static_cast<SQLSMALLINT>(
            Type_ == EDescType::Explicit ? SQL_DESC_ALLOC_USER : SQL_DESC_ALLOC_AUTO));
    }
    if (field == SQL_DESC_COUNT) {
        return WriteScalar(value, GetRecordCount());
    }
    if (auto result = THeaderProperties::Get(field, Header_, value)) {
        return *result;
    }

    const TDescRecord* record = FindRecord(recNumber);
    if (!record) {
        return recNumber > GetRecordCount()
            ? SQL_NO_DATA
            : AddError("07009", 0, "Invalid descriptor index");
    }
    if (auto result = TRecordProperties::Get(field, *record, value)) {
        return *result;
    }
    switch (field) {
        case SQL_DESC_BASE_COLUMN_NAME:
        case SQL_DESC_NAME:
            return Diag::WriteString<Diag::EStringWriteMode::Descriptor>(
                this, record->Name, value, bufferLength, lengthPtr);
        case SQL_DESC_TYPE_NAME:
        case SQL_DESC_LOCAL_TYPE_NAME:
            return Diag::WriteString<Diag::EStringWriteMode::Descriptor>(
                this, TypeName(record->Type), value, bufferLength, lengthPtr);
        case SQL_DESC_LITERAL_PREFIX:
        case SQL_DESC_LITERAL_SUFFIX:
            return Diag::WriteString<Diag::EStringWriteMode::Descriptor>(this,
                IsCharacter(record->Type) || DateTimeCode(record->Type) ? "'" : "",
                value, bufferLength, lengthPtr);
        case SQL_DESC_TYPE:
            return WriteScalar(value, static_cast<SQLSMALLINT>(
                DateTimeCode(record->Type) ? SQL_DATETIME : record->Type));
        case SQL_DESC_DATETIME_INTERVAL_CODE:
            return WriteScalar(value, DateTimeCode(record->Type));
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
    SQLSMALLINT nameLength = 0;
    SQLRETURN result = SQL_SUCCESS;
    if (name) {
        result = Diag::WriteString<Diag::EStringWriteMode::Descriptor>(
            this, record->Name, name, bufferLength, &nameLength);
    } else {
        nameLength = static_cast<SQLSMALLINT>(record->Name.size());
    }
    WriteIf(nameLengthPtr, nameLength);
    WriteIf(typePtr, DateTimeCode(record->Type) ? SQL_DATETIME : record->Type);
    WriteIf(subTypePtr, DateTimeCode(record->Type));
    WriteIf(lengthPtr, record->Length);
    WriteIf(precisionPtr, record->Precision);
    WriteIf(scalePtr, record->Scale);
    WriteIf(nullablePtr, record->Nullable);
    return result;
}

SQLRETURN TDescriptor::SetDescField(SQLSMALLINT recNumber, SQLSMALLINT field, SQLPOINTER value,
                                    SQLINTEGER bufferLength) {
    switch (field) {
        case SQL_DESC_COUNT: {
            const auto count = static_cast<SQLSMALLINT>(reinterpret_cast<intptr_t>(value));
            if (count < 0) {
                return AddError("HY024", 0, "Invalid SQL_DESC_COUNT value");
            }
            Records_.resize(static_cast<size_t>(count));
            Changed();
            for (auto& record : Records_) {
                record.Active = true;
            }
            return SQL_SUCCESS;
        }
        case SQL_DESC_ARRAY_SIZE: {
            const auto size = static_cast<SQLULEN>(reinterpret_cast<uintptr_t>(value));
            if (size == 0) {
                return AddError("HY024", 0, "Invalid SQL_DESC_ARRAY_SIZE value");
            }
            Header_.ArraySize = size;
            Changed(false);
            return SQL_SUCCESS;
        }
        default: break;
    }
    if (THeaderProperties::Set(field, Header_, value)) {
        Changed(false);
        return SQL_SUCCESS;
    }

    if (Type_ == EDescType::ImpRow) {
        return AddError("HY016", 0, "Cannot modify an implementation row descriptor");
    }

    if (TRecordProperties::CanSet(field)) {
        TRecordProperties::Set(field, Record(recNumber), value);
        return SQL_SUCCESS;
    }
    if (field == SQL_DESC_NAME) {
        if (!value) {
            return Diag::AddNullPointer(*this);
        }
        if (bufferLength < 0 && bufferLength != SQL_NTS) {
            return Diag::AddInvalidBufferLength(*this);
        }
        std::string name = bufferLength == SQL_NTS
            ? std::string(static_cast<const char*>(value))
            : std::string(static_cast<const char*>(value), static_cast<size_t>(bufferLength));
        Record(recNumber).Name = std::move(name);
        return SQL_SUCCESS;
    }
    return Diag::AddNotImplemented(*this);
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

SQLRETURN TDescriptor::Copy(SQLHDESC source, SQLHDESC target) {
    auto src = std::dynamic_pointer_cast<TDescriptor>(PinHandle(source));
    auto dst = std::dynamic_pointer_cast<TDescriptor>(PinHandle(target));
    if (!src || !dst) {
        return SQL_INVALID_HANDLE;
    }
    std::shared_lock sourceLifecycle(*src->GetLifecycle(), std::defer_lock);
    std::shared_lock<std::shared_mutex> targetLifecycle;
    if (dst->GetLifecycle() != src->GetLifecycle()) {
        targetLifecycle = std::shared_lock(*dst->GetLifecycle(), std::defer_lock);
        std::lock(sourceLifecycle, targetLifecycle);
    } else {
        sourceLifecycle.lock();
    }
    std::unique_lock sourceLock(src->OperationMutex_, std::defer_lock);
    std::unique_lock targetLock(dst->OperationMutex_, std::defer_lock);
    if (src != dst) {
        std::lock(sourceLock, targetLock);
    } else {
        sourceLock.lock();
    }
    if (src->IsRetired() || dst->IsRetired()) {
        return SQL_INVALID_HANDLE;
    }
    src->ClearErrors();
    try {
        const auto result = src->CopyDesc(*dst);
        src->SetLastReturnCode(result);
        return result;
    } catch (...) {
        return RecordCurrentException(*src);
    }
}

SQLRETURN TDescriptor::CopyDesc(TDescriptor& target) {
    if (target.Type_ == EDescType::ImpRow) {
        return AddError("HY016", 0, "Cannot modify an implementation row descriptor");
    }
    target.Changed();
    target.Header_ = Header_;
    target.Records_ = Records_;
    return SQL_SUCCESS;
}

} // namespace NYdb::NOdbc
