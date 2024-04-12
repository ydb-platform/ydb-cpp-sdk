#pragma once

#include <string_view>

std::string_view UnescapeJsonUnicode(std::string_view data, char* scratch);
