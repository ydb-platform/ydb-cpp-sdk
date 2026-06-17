#include "descriptor.h"
#include "statement.h"
#include "utils/param_rewrite.h"

#include <algorithm>
#include <cstring>

namespace NYdb {
namespace NOdbc {

TDescriptor::TDescriptor(EDescType type, TStatement* stmt)
    : Type_(type)
    , Stmt_(stmt) {}

TDescriptor* TDescriptor::FromHandle(SQLHDESC handle) {
    if (!handle) {
        throw TOdbcException("HY000", 0, "Invalid handle", SQL_INVALID_HANDLE);
    }
    return static_cast<TDescriptor*>(handle);
}

SQLSMALLINT TDescriptor::GetRecordCount() const {
    if (Type_ == EDescType::Explicit) {
        return static_cast<SQLSMALLINT>(ExplicitRecs_.size());
    }
    if (!Stmt_) {
        return 0;
    }
    switch (Type_) {
        case EDescType::ImpRow: {
            const auto& cols = Stmt_->GetColumnMeta();
            return static_cast<SQLSMALLINT>(cols.size());
        }
        case EDescType::AppRow: {
            SQLSMALLINT maxRec = 0;
            for (const auto& col : Stmt_->BoundColumns_) {
                maxRec = std::max(maxRec, static_cast<SQLSMALLINT>(col.ColumnNumber));
            }
            return maxRec;
        }
        case EDescType::ImpParam:
            return Stmt_->IsPrepared_ ? Stmt_->GetParamCount() : 0;
        case EDescType::AppParam: {
            SQLSMALLINT maxRec = 0;
            for (const auto& param : Stmt_->BoundParams_) {
                maxRec = std::max(maxRec, static_cast<SQLSMALLINT>(param.ParamNumber));
            }
            return maxRec;
        }
        default:
            return 0;
    }
}

bool TDescriptor::GetExplicitRecord(SQLSMALLINT recNumber, TExplicitDescRec& out) const {
    if (recNumber < 1 || static_cast<size_t>(recNumber) > ExplicitRecs_.size()) {
        return false;
    }
    out = ExplicitRecs_[static_cast<size_t>(recNumber - 1)];
    return true;
}

TExplicitDescRec& TDescriptor::GetOrCreateExplicitRecord(SQLSMALLINT recNumber) {
    if (recNumber < 1) {
        throw TOdbcException("07009", 0, "Invalid descriptor index");
    }
    const size_t idx = static_cast<size_t>(recNumber - 1);
    if (ExplicitRecs_.size() <= idx) {
        ExplicitRecs_.resize(idx + 1);
    }
    return ExplicitRecs_[idx];
}

SQLRETURN TDescriptor::GetDescField(SQLSMALLINT recNumber, SQLSMALLINT fieldIdentifier, SQLPOINTER value,
                                    SQLINTEGER bufferLength, SQLINTEGER* stringLengthPtr) {
    if (!value && fieldIdentifier != SQL_DESC_ROWS_PROCESSED_PTR) {
        return AddError("HY009", 0, "Invalid use of null pointer");
    }
    switch (fieldIdentifier) {
        case SQL_DESC_ALLOC_TYPE:
            *reinterpret_cast<SQLSMALLINT*>(value) =
                Type_ == EDescType::Explicit ? SQL_DESC_ALLOC_USER : SQL_DESC_ALLOC_AUTO;
            return SQL_SUCCESS;
        case SQL_DESC_COUNT:
            *reinterpret_cast<SQLSMALLINT*>(value) = GetRecordCount();
            return SQL_SUCCESS;
        case SQL_DESC_ARRAY_SIZE:
            *reinterpret_cast<SQLULEN*>(value) = 1;
            return SQL_SUCCESS;
        case SQL_DESC_TYPE:
        case SQL_DESC_CONCISE_TYPE:
        case SQL_DESC_LENGTH:
        case SQL_DESC_PRECISION:
        case SQL_DESC_SCALE:
        case SQL_DESC_NULLABLE:
        case SQL_DESC_NAME: {
            SQLCHAR name[256] = {};
            SQLSMALLINT nameLen = 0;
            SQLSMALLINT type = 0;
            SQLSMALLINT subType = 0;
            SQLLEN length = 0;
            SQLSMALLINT precision = 0;
            SQLSMALLINT scale = 0;
            SQLSMALLINT nullable = 0;
            const SQLRETURN rc = GetDescRec(recNumber, name, sizeof(name), &nameLen, &type, &subType,
                                            &length, &precision, &scale, &nullable);
            if (rc != SQL_SUCCESS) {
                return rc;
            }
            if (fieldIdentifier == SQL_DESC_NAME) {
                if (stringLengthPtr) {
                    *stringLengthPtr = nameLen;
                }
                if (bufferLength > 0) {
                    const SQLINTEGER copyLen = std::min<SQLINTEGER>(nameLen, bufferLength - 1);
                    std::memcpy(value, name, static_cast<size_t>(copyLen));
                    reinterpret_cast<char*>(value)[copyLen] = '\0';
                }
                return SQL_SUCCESS;
            }
            if (fieldIdentifier == SQL_DESC_TYPE || fieldIdentifier == SQL_DESC_CONCISE_TYPE) {
                *reinterpret_cast<SQLSMALLINT*>(value) = type;
            } else if (fieldIdentifier == SQL_DESC_LENGTH) {
                *reinterpret_cast<SQLLEN*>(value) = length;
            } else if (fieldIdentifier == SQL_DESC_PRECISION) {
                *reinterpret_cast<SQLSMALLINT*>(value) = precision;
            } else if (fieldIdentifier == SQL_DESC_SCALE) {
                *reinterpret_cast<SQLSMALLINT*>(value) = scale;
            } else if (fieldIdentifier == SQL_DESC_NULLABLE) {
                *reinterpret_cast<SQLSMALLINT*>(value) = nullable;
            }
            return SQL_SUCCESS;
        }
        default:
            return AddError("HYC00", 0, "Optional feature not implemented");
    }
}

SQLRETURN TDescriptor::GetDescRec(SQLSMALLINT recNumber, SQLCHAR* name, SQLSMALLINT bufferLength,
                                  SQLSMALLINT* stringLengthPtr, SQLSMALLINT* typePtr, SQLSMALLINT* subTypePtr,
                                  SQLLEN* lengthPtr, SQLSMALLINT* precisionPtr, SQLSMALLINT* scalePtr,
                                  SQLSMALLINT* nullablePtr) {
    if (recNumber < 1) {
        return AddError("07009", 0, "Invalid descriptor index");
    }
    if (Type_ == EDescType::Explicit) {
        TExplicitDescRec rec;
        if (!GetExplicitRecord(recNumber, rec)) {
            return AddError("07009", 0, "Invalid descriptor index");
        }
        if (stringLengthPtr) {
            *stringLengthPtr = static_cast<SQLSMALLINT>(rec.Name.size());
        }
        if (name && bufferLength > 0) {
            const auto copyLen = std::min<size_t>(rec.Name.size(), static_cast<size_t>(bufferLength - 1));
            std::memcpy(name, rec.Name.data(), copyLen);
            name[copyLen] = '\0';
        }
        if (typePtr) {
            *typePtr = rec.Type;
        }
        if (subTypePtr) {
            *subTypePtr = rec.SubType;
        }
        if (lengthPtr) {
            *lengthPtr = rec.Length;
        }
        if (precisionPtr) {
            *precisionPtr = rec.Precision;
        }
        if (scalePtr) {
            *scalePtr = rec.Scale;
        }
        if (nullablePtr) {
            *nullablePtr = rec.Nullable;
        }
        return SQL_SUCCESS;
    }
    if (!Stmt_) {
        return AddError("HY000", 0, "Invalid descriptor");
    }
    if (Type_ == EDescType::ImpRow) {
        const auto& cols = Stmt_->GetColumnMeta();
        if (static_cast<size_t>(recNumber) > cols.size()) {
            return AddError("07009", 0, "Invalid descriptor index");
        }
        const auto& col = cols[static_cast<size_t>(recNumber - 1)];
        if (stringLengthPtr) {
            *stringLengthPtr = static_cast<SQLSMALLINT>(col.Name.size());
        }
        if (name && bufferLength > 0) {
            const auto copyLen = std::min<size_t>(col.Name.size(), static_cast<size_t>(bufferLength - 1));
            std::memcpy(name, col.Name.data(), copyLen);
            name[copyLen] = '\0';
        }
        if (typePtr) {
            *typePtr = col.SqlType;
        }
        if (subTypePtr) {
            *subTypePtr = 0;
        }
        if (lengthPtr) {
            *lengthPtr = static_cast<SQLLEN>(col.Size);
        }
        if (precisionPtr) {
            *precisionPtr = static_cast<SQLSMALLINT>(col.Size);
        }
        if (scalePtr) {
            *scalePtr = col.DecimalDigits;
        }
        if (nullablePtr) {
            *nullablePtr = col.Nullable;
        }
        return SQL_SUCCESS;
    }
    if (Type_ == EDescType::AppRow) {
        const auto it = std::find_if(Stmt_->BoundColumns_.begin(), Stmt_->BoundColumns_.end(),
            [recNumber](const TBoundColumn& col) { return col.ColumnNumber == static_cast<SQLUSMALLINT>(recNumber); });
        if (it == Stmt_->BoundColumns_.end()) {
            return AddError("07009", 0, "Invalid descriptor index");
        }
        if (stringLengthPtr) {
            *stringLengthPtr = 0;
        }
        if (typePtr) {
            *typePtr = it->TargetType;
        }
        if (subTypePtr) {
            *subTypePtr = 0;
        }
        if (lengthPtr) {
            *lengthPtr = it->BufferLength;
        }
        if (precisionPtr) {
            *precisionPtr = 0;
        }
        if (scalePtr) {
            *scalePtr = 0;
        }
        if (nullablePtr) {
            *nullablePtr = SQL_NULLABLE;
        }
        return SQL_SUCCESS;
    }
    if (Type_ == EDescType::AppParam || Type_ == EDescType::ImpParam) {
        const auto it = std::find_if(Stmt_->BoundParams_.begin(), Stmt_->BoundParams_.end(),
            [recNumber](const TBoundParam& p) { return p.ParamNumber == static_cast<SQLUSMALLINT>(recNumber); });
        if (it != Stmt_->BoundParams_.end()) {
            if (stringLengthPtr) {
                *stringLengthPtr = 0;
            }
            if (typePtr) {
                *typePtr = Type_ == EDescType::AppParam ? it->ValueType : it->ParameterType;
            }
            if (subTypePtr) {
                *subTypePtr = 0;
            }
            if (lengthPtr) {
                *lengthPtr = static_cast<SQLLEN>(it->ColumnSize);
            }
            if (precisionPtr) {
                *precisionPtr = static_cast<SQLSMALLINT>(it->ColumnSize);
            }
            if (scalePtr) {
                *scalePtr = it->DecimalDigits;
            }
            if (nullablePtr) {
                *nullablePtr = SQL_NULLABLE;
            }
            return SQL_SUCCESS;
        }
        if (Type_ == EDescType::ImpParam && Stmt_->IsPrepared_
            && recNumber <= CountOdbcParams(Stmt_->PreparedQuery_)) {
            if (stringLengthPtr) {
                *stringLengthPtr = 0;
            }
            if (typePtr) {
                *typePtr = SQL_UNKNOWN_TYPE;
            }
            if (subTypePtr) {
                *subTypePtr = 0;
            }
            if (lengthPtr) {
                *lengthPtr = 0;
            }
            if (precisionPtr) {
                *precisionPtr = 0;
            }
            if (scalePtr) {
                *scalePtr = 0;
            }
            if (nullablePtr) {
                *nullablePtr = SQL_NULLABLE_UNKNOWN;
            }
            return SQL_SUCCESS;
        }
        return AddError("07009", 0, "Invalid descriptor index");
    }
    return AddError("HYC00", 0, "Optional feature not implemented");
}

SQLRETURN TDescriptor::SetDescField(SQLSMALLINT recNumber, SQLSMALLINT fieldIdentifier, SQLPOINTER value,
                                      SQLINTEGER bufferLength) {
    (void)bufferLength;
    if (Type_ != EDescType::Explicit) {
        return AddError("HY017", 0, "Invalid use of an automatically allocated descriptor handle");
    }
    auto& rec = GetOrCreateExplicitRecord(recNumber);
    switch (fieldIdentifier) {
        case SQL_DESC_TYPE:
        case SQL_DESC_CONCISE_TYPE:
            rec.Type = *reinterpret_cast<SQLSMALLINT*>(value);
            return SQL_SUCCESS;
        case SQL_DESC_LENGTH:
            rec.Length = *reinterpret_cast<SQLLEN*>(value);
            return SQL_SUCCESS;
        case SQL_DESC_PRECISION:
            rec.Precision = *reinterpret_cast<SQLSMALLINT*>(value);
            return SQL_SUCCESS;
        case SQL_DESC_SCALE:
            rec.Scale = *reinterpret_cast<SQLSMALLINT*>(value);
            return SQL_SUCCESS;
        case SQL_DESC_NULLABLE:
            rec.Nullable = *reinterpret_cast<SQLSMALLINT*>(value);
            return SQL_SUCCESS;
        default:
            return AddError("HYC00", 0, "Optional feature not implemented");
    }
}

SQLRETURN TDescriptor::SetDescRec(SQLSMALLINT recNumber, SQLSMALLINT type, SQLSMALLINT subType, SQLLEN length,
                                  SQLSMALLINT precision, SQLSMALLINT scale, SQLPOINTER dataPtr,
                                  SQLLEN* stringLengthPtr, SQLLEN* indicatorPtr) {
    if (Type_ == EDescType::Explicit) {
        auto& rec = GetOrCreateExplicitRecord(recNumber);
        rec.Type = type;
        rec.SubType = subType;
        rec.Length = length;
        rec.Precision = precision;
        rec.Scale = scale;
        rec.DataPtr = dataPtr;
        rec.Indicator = indicatorPtr ? *indicatorPtr : 0;
        return SQL_SUCCESS;
    }
    if (!Stmt_) {
        return AddError("HY000", 0, "Invalid descriptor");
    }
    if (Type_ == EDescType::AppRow) {
        return Stmt_->BindCol(static_cast<SQLUSMALLINT>(recNumber), type, dataPtr, length, indicatorPtr);
    }
    if (Type_ == EDescType::AppParam) {
        return Stmt_->BindParameter(static_cast<SQLUSMALLINT>(recNumber), SQL_PARAM_INPUT, type, subType,
                                     static_cast<SQLULEN>(length), scale, dataPtr, length, indicatorPtr);
    }
    return AddError("HY017", 0, "Invalid use of an automatically allocated descriptor handle");
}

SQLRETURN TDescriptor::CopyDesc(TDescriptor* target) {
    if (!target) {
        return AddError("HY009", 0, "Invalid use of null pointer");
    }
    if (Type_ != EDescType::Explicit || target->Type_ != EDescType::Explicit) {
        return AddError("HYC00", 0, "Optional feature not implemented");
    }
    target->ExplicitRecs_ = ExplicitRecs_;
    return SQL_SUCCESS;
}

} // namespace NOdbc
} // namespace NYdb
