#pragma once

#include <ydb-cpp-sdk/util/system/types.h>

class IOutputStream;

void HexEncode(const void* in, size_t len, IOutputStream& out);
void HexDecode(const void* in, size_t len, IOutputStream& out);
