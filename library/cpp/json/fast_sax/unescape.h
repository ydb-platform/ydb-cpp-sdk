#pragma once

#include <util/generic/strbuf.h>

std::string_view UnescapeJsonUnicode(std::string_view data, char* scratch);
