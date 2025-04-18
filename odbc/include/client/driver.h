#pragma once

#include <stddef.h> // для size_t

#ifdef __cplusplus
extern "C" {
#endif

// Функции для работы с YDB через C++ SDK
void* YDB_CreateDriver(const char* endpoint, const char* user, const char* password);
void YDB_DestroyDriver(void* driver);

#ifdef __cplusplus
}
#endif
