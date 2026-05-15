#pragma once

#include <cstdint>
#include <string>

std::string SloJoinPath(const std::string& prefix, const std::string& path);
std::string SloGetDatabaseFromConnectionString(const std::string& connectionString);
std::string SloGenerateRandomString(std::uint32_t minLength, std::uint32_t maxLength);
