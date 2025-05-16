#include "statement.h"

#include <cstring>
#include <ydb-cpp-sdk/client/params/params.h>
#include <ydb-cpp-sdk/client/value/value.h>

namespace NYdb {
namespace NOdbc {

TStatement::TStatement(TConnection* conn)
    : Conn_(conn) {}

SQLRETURN TStatement::ExecDirect(const std::string& statementText) {
    ClearStatement();

    auto* client = Conn_->GetClient();
    if (!client) {
        return SQL_ERROR;
    }

    NYdb::TParams params = BuildParams();
    if (!Errors_.empty()) {
        return SQL_ERROR;
    }
    // --- конец сборки параметров ---

    auto sessionResult = client->GetSession().ExtractValueSync();
    if (!sessionResult.IsSuccess()) {
        return SQL_ERROR;
    }

    auto session = sessionResult.GetSession();

    auto iterator = session.StreamExecuteQuery(statementText, NYdb::NQuery::TTxControl::NoTx(), params).ExtractValueSync();
    if (!iterator.IsSuccess()) {
        return SQL_ERROR;
    }

    Iterator_ = std::make_unique<NYdb::NQuery::TExecuteQueryIterator>(std::move(iterator));

    return SQL_SUCCESS;
}

SQLRETURN TStatement::Fetch() {
    if (!Iterator_) {
        ClearStatement();
        return SQL_NO_DATA;
    }

    while (true) {
        if (ResultSetParser_) {
            if (ResultSetParser_->TryNextRow()) {
                // Автоматически заполняем связанные буферы
                for (const auto& col : BoundColumns_) {
                    GetData(col.ColumnNumber, col.TargetType, col.TargetValue, col.BufferLength, col.StrLenOrInd);
                }
                return SQL_SUCCESS;
            }

            ResultSetParser_.reset();
        }

        auto part = Iterator_->ReadNext().ExtractValueSync();
        if (part.EOS()) {
            ClearStatement();
            return SQL_NO_DATA;
        }

        if (!part.IsSuccess()) {
            // AddError(part.GetStatus().GetStatus().GetCode(), part.GetStatus().GetStatus().GetReason());
            ClearStatement();
            return SQL_ERROR;
        }

        if (part.HasResultSet()) {
            ResultSetParser_ = std::make_unique<TResultSetParser>(part.ExtractResultSet());
        }
    }

    return SQL_SUCCESS;
}

SQLRETURN TStatement::GetData(SQLUSMALLINT columnNumber, SQLSMALLINT targetType, 
                              SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd) {
    if (!ResultSetParser_) {
        return SQL_NO_DATA;
    }

    if (columnNumber < 1 || columnNumber > ResultSetParser_->ColumnsCount()) {
        return SQL_ERROR;
    }

    return ConvertYdbValue(ResultSetParser_->ColumnParser(columnNumber - 1), targetType, targetValue, bufferLength, strLenOrInd);
}

SQLRETURN TStatement::GetDiagRec(SQLSMALLINT recNumber, SQLCHAR* sqlState, SQLINTEGER* nativeError, 
                                SQLCHAR* messageText, SQLSMALLINT bufferLength, SQLSMALLINT* textLength) {
    if (recNumber < 1 || recNumber > (SQLSMALLINT)Errors_.size()) return SQL_NO_DATA;
    const auto& err = Errors_[recNumber-1];
    if (sqlState) strncpy((char*)sqlState, err.SqlState.c_str(), 6);
    if (nativeError) *nativeError = err.NativeError;
    if (messageText && bufferLength > 0) {
        strncpy((char*)messageText, err.Message.c_str(), bufferLength);
        if (textLength) *textLength = (SQLSMALLINT)std::min((int)err.Message.size(), (int)bufferLength);
    }
    return SQL_SUCCESS;
}

SQLRETURN TStatement::BindCol(SQLUSMALLINT columnNumber, SQLSMALLINT targetType, SQLPOINTER targetValue, SQLLEN bufferLength, SQLLEN* strLenOrInd) {
    // Удаляем старую связь для этой колонки, если есть
    BoundColumns_.erase(std::remove_if(BoundColumns_.begin(), BoundColumns_.end(),
        [columnNumber](const TBoundColumn& col) { return col.ColumnNumber == columnNumber; }), BoundColumns_.end());
    // Если targetValue == nullptr, просто удаляем связь
    if (!targetValue) {
        return SQL_SUCCESS;
    }
    BoundColumns_.push_back({columnNumber, targetType, targetValue, bufferLength, strLenOrInd});
    return SQL_SUCCESS;
}

SQLRETURN TStatement::BindParameter(SQLUSMALLINT paramNumber,
                                    SQLSMALLINT inputOutputType,
                                    SQLSMALLINT valueType,
                                    SQLSMALLINT parameterType,
                                    SQLULEN columnSize,
                                    SQLSMALLINT decimalDigits,
                                    SQLPOINTER parameterValuePtr,
                                    SQLLEN bufferLength,
                                    SQLLEN* strLenOrIndPtr) {
    if (inputOutputType != SQL_PARAM_INPUT) {
        AddError("HYC00", 0, "Only input parameters are supported");
        return SQL_ERROR;
    }
    // Удаляем старую связь для этого параметра, если есть
    BoundParams_.erase(std::remove_if(BoundParams_.begin(), BoundParams_.end(),
        [paramNumber](const TBoundParam& p) { return p.ParamNumber == paramNumber; }), BoundParams_.end());
    // Если parameterValuePtr == nullptr, просто удаляем связь
    if (!parameterValuePtr) {
        return SQL_SUCCESS;
    }
    BoundParams_.push_back({paramNumber, inputOutputType, valueType, parameterType, columnSize, decimalDigits, parameterValuePtr, bufferLength, strLenOrIndPtr});
    return SQL_SUCCESS;
}

void TStatement::AddError(const std::string& sqlState, SQLINTEGER nativeError, const std::string& message) {
    Errors_.push_back({sqlState, nativeError, message});
}

void TStatement::ClearErrors() {
    Errors_.clear();
}

void TStatement::ClearStatement() {
    Iterator_.reset();
    ResultSetParser_.reset();
    BoundColumns_.clear();
}

SQLRETURN TStatement::ConvertYdbValue(NYdb::TValueParser& valueParser,
                                      SQLSMALLINT targetType,
                                      SQLPOINTER targetValue,
                                      SQLLEN bufferLength,
                                      SQLLEN* strLenOrInd) {

    // 1. Проверка на NULL
    if (valueParser.IsNull()) {
        if (strLenOrInd) *strLenOrInd = SQL_NULL_DATA;
        return SQL_SUCCESS;
    }

    if (valueParser.GetKind() == TTypeParser::ETypeKind::Optional) {
        valueParser.OpenOptional();
        SQLRETURN ret = ConvertYdbValue(valueParser, targetType, targetValue, bufferLength, strLenOrInd);
        valueParser.CloseOptional();
        return ret;
    }

    if (valueParser.GetKind() != TTypeParser::ETypeKind::Primitive) {
        return SQL_ERROR;
    }

    EPrimitiveType ydbType = valueParser.GetPrimitiveType();

    switch (targetType) {
        case SQL_C_SLONG:
        {
            int32_t v = 0;
            switch (ydbType) {
                case EPrimitiveType::Int32: v = valueParser.GetInt32(); break;
                case EPrimitiveType::Uint32: v = static_cast<int32_t>(valueParser.GetUint32()); break;
                case EPrimitiveType::Int64: v = static_cast<int32_t>(valueParser.GetInt64()); break;
                case EPrimitiveType::Uint64: v = static_cast<int32_t>(valueParser.GetUint64()); break;
                case EPrimitiveType::Bool: v = valueParser.GetBool() ? 1 : 0; break;
                default: return SQL_ERROR;
            }
            if (targetValue) *reinterpret_cast<int32_t*>(targetValue) = v;
            if (strLenOrInd) *strLenOrInd = sizeof(int32_t);
            return SQL_SUCCESS;
        }
        case SQL_C_SBIGINT:
        {
            SQLBIGINT v = 0;
            switch (ydbType) {
                case EPrimitiveType::Int64: v = valueParser.GetInt64(); break;
                case EPrimitiveType::Uint64: v = static_cast<SQLBIGINT>(valueParser.GetUint64()); break;
                case EPrimitiveType::Int32: v = static_cast<SQLBIGINT>(valueParser.GetInt32()); break;
                case EPrimitiveType::Uint32: v = static_cast<SQLBIGINT>(valueParser.GetUint32()); break;
                default: return SQL_ERROR;
            }
            if (targetValue) *reinterpret_cast<SQLBIGINT*>(targetValue) = v;
            if (strLenOrInd) *strLenOrInd = sizeof(SQLBIGINT);
            return SQL_SUCCESS;
        }
        case SQL_C_DOUBLE:
        {
            double v = 0.0;
            switch (ydbType) {
                case EPrimitiveType::Double: v = valueParser.GetDouble(); break;
                case EPrimitiveType::Float: v = valueParser.GetFloat(); break;
                default: return SQL_ERROR;
            }
            if (targetValue) *reinterpret_cast<double*>(targetValue) = v;
            if (strLenOrInd) *strLenOrInd = sizeof(double);
            return SQL_SUCCESS;
        }
        case SQL_C_CHAR:
        {
            std::string str;
            switch (ydbType) {
                case EPrimitiveType::Utf8: str = valueParser.GetUtf8(); break;
                case EPrimitiveType::String: str = valueParser.GetString(); break;
                case EPrimitiveType::Json: str = valueParser.GetJson(); break;
                case EPrimitiveType::JsonDocument: str = valueParser.GetJsonDocument(); break;
                default: return SQL_ERROR;
            }
            SQLLEN len = str.size();
            if (targetValue && bufferLength > 0) {
                SQLLEN copyLen = std::min(len, bufferLength - 1);
                memcpy(targetValue, str.data(), copyLen);
                reinterpret_cast<char*>(targetValue)[copyLen] = 0;
            }
            if (strLenOrInd) *strLenOrInd = len;
            return SQL_SUCCESS;
        }
        case SQL_C_BIT:
        {
            char v = valueParser.GetBool() ? 1 : 0;
            if (targetValue) *reinterpret_cast<char*>(targetValue) = v;
            if (strLenOrInd) *strLenOrInd = sizeof(char);
            return SQL_SUCCESS;
        }
        // Добавьте обработку дат/времени, бинарных данных и других типов по необходимости
        default:
            return SQL_ERROR;
    }
}

NYdb::TParams TStatement::BuildParams() {
    Errors_.clear();
    NYdb::TParamsBuilder paramsBuilder;
    for (const auto& param : BoundParams_) {
        std::string paramName = "$p" + std::to_string(param.ParamNumber); // ODBC нумерует с 1
        auto& builder = paramsBuilder.AddParam(paramName);
        // Обработка NULL
        if (param.StrLenOrIndPtr && *param.StrLenOrIndPtr == SQL_NULL_DATA) {
            builder.EmptyOptional();
            builder.Build();
            continue;
        }

        switch (param.ValueType) {
            case SQL_C_SLONG: {
                auto value = *static_cast<SQLINTEGER*>(param.ParameterValuePtr);
                switch (param.ParameterType) {
                    case SQL_INTEGER:
                        builder.Int32(static_cast<int32_t>(value));
                        break;
                    case SQL_BIGINT:
                        builder.Int64(static_cast<int64_t>(value));
                        break;
                    case SQL_DOUBLE:
                        builder.Double(static_cast<double>(value));
                        break;
                    case SQL_FLOAT:
                        builder.Float(static_cast<float>(value));
                        break;
                    case SQL_VARCHAR:
                    case SQL_CHAR:
                    case SQL_LONGVARCHAR:
                        builder.Utf8(std::to_string(value));
                        break;
                    case SQL_BIT:
                        builder.Uint8(static_cast<uint8_t>(value));
                        break;
                    default:
                        AddError("07006", 0, "Unsupported SQL type");
                        return paramsBuilder.Build();
                }
                break;
            }
            case SQL_C_SBIGINT: {
                auto v = *static_cast<SQLBIGINT*>(param.ParameterValuePtr);
                builder.Int32(static_cast<int32_t>(v));
                break;
            }
            default: {
                AddError("07006", 0, "Unsupported C type");
                return paramsBuilder.Build();
            }
        }

        switch (param.ParameterType) {
            case SQL_INTEGER:
            case SQL_BIGINT:
                break;
            case SQL_DOUBLE:
                builder.Double(*reinterpret_cast<double*>(param.ParameterValuePtr));
                break;
            case SQL_FLOAT:
                builder.Double(*reinterpret_cast<double*>(param.ParameterValuePtr));
                break;
            case SQL_VARCHAR:
            case SQL_CHAR:
            case SQL_LONGVARCHAR:
                builder.Utf8(*reinterpret_cast<std::string*>(param.ParameterValuePtr));
                break;
            case SQL_BIT:
                builder.Bool(*reinterpret_cast<bool*>(param.ParameterValuePtr));
                break;
            default:
                AddError("07006", 0, "Unsupported SQL type");
                return paramsBuilder.Build();
        }

        builder.Build();
    }

    return paramsBuilder.Build();
}

} // namespace NOdbc
} // namespace NYdb 