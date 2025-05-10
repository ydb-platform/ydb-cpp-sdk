#pragma once

#include <ydb-cpp-sdk/c_api/value.h>

#include <ydb-cpp-sdk/client/value/value.h>

#include <optional>
#include <string>

struct TYdbValueImpl {
    EYdbValueStatus errorCode;
    std::string errorMessage;

    std::optional<NYdb::TValue> value;
};
