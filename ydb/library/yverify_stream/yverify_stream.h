#pragma once

#include <util/system/yassert.h>
#include <util/string/builder.h>

#define Y_VERIFY(...) Y_ABORT_UNLESS(__VA_ARGS__)
#define Y_FAIL(...) Y_ABORT(__VA_ARGS__)
#define Y_VERIFY_DEBUG(...) Y_DEBUG_ABORT_UNLESS(__VA_ARGS__)

#define Y_VERIFY_S(expr, msg) Y_VERIFY(expr, "%s", (TYdbStringBuilder() << msg).c_str())
#define Y_FAIL_S(msg) Y_FAIL("%s", (TYdbStringBuilder() << msg).c_str())
#define Y_VERIFY_DEBUG_S(expr, msg) Y_VERIFY_DEBUG(expr, "%s", (TYdbStringBuilder() << msg).c_str())
