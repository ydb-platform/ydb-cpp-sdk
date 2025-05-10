#pragma once

#include <ydb-cpp-sdk/c_api/result.h>

#include <ydb-cpp-sdk/client/result/result.h>

#include <optional>

struct TYdbResultSetImpl {
    EYdbResultSetStatus errorCode;
    std::string errorMessage;

    std::optional<NYdb::TResultSet> result;
};
