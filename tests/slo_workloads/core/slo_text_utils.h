#pragma once

#include <ydb-cpp-sdk/client/types/status/status.h>

#include <cstdint>
#include <string>

std::string SloJoinPath(const std::string& prefix, const std::string& path);
std::string SloGetDatabaseFromConnectionString(const std::string& connectionString);
std::string SloGenerateRandomString(std::uint32_t minLength, std::uint32_t maxLength);
std::string SloYdbStatusToString(NYdb::EStatus status);
NYdb::EStatus SloYdbStatusFromString(const std::string& label);
