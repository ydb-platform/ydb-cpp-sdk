#pragma once

#include <stddef.h>
#include <ctype.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(__FreeBSD__) && !defined(__APPLE__)
size_t strlcpy(char* dst, const char* src, size_t len);
size_t strlcat(char* dst, const char* src, size_t len);
#endif

#if !defined(_MSC_VER)

#define stricmp strcasecmp
#define strnicmp strncasecmp

char* strlwr(char*);

#else // _MSC_VER

#define strcasecmp stricmp
#define strncasecmp strnicmp

#endif // _MSC_VER

#ifdef __cplusplus
} //extern "C"
#endif
