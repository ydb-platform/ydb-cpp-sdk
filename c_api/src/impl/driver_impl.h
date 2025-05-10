#pragma once

#include <ydb-cpp-sdk/c_api/driver.h>

#include <ydb-cpp-sdk/client/driver/driver.h>

#include <optional>
#include <string>

struct TYdbDriverConfigImpl {
    EYdbDriverConfigStatus errorCode;
    std::string errorMessage;

    NYdb::TDriverConfig config;
};

struct TYdbDriverImpl {
    EYdbDriverStatus errorCode;
    std::string errorMessage;

    std::optional<NYdb::TDriver> driver;
};
