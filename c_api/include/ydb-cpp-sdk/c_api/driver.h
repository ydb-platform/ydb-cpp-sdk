#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TYdbDriverConfigImpl TYdbDriverConfig;
typedef struct TYdbDriverImpl TYdbDriver;

typedef enum {
    YDB_DRIVER_CONFIG_OK,
    YDB_DRIVER_CONFIG_INVALID,
} EYdbDriverConfigStatus;

typedef enum {
    YDB_DRIVER_OK,
    YDB_DRIVER_ERROR,
} EYdbDriverStatus;

// Создание и уничтожение конфигурации
TYdbDriverConfig* YdbCreateDriverConfig(const char* connectionString);
void YdbDestroyDriverConfig(TYdbDriverConfig* config);

// Установка параметров конфигурации
TYdbDriverConfig* YdbSetEndpoint(TYdbDriverConfig* config, const char* endpoint);
TYdbDriverConfig* YdbSetDatabase(TYdbDriverConfig* config, const char* database);
TYdbDriverConfig* YdbSetAuthToken(TYdbDriverConfig* config, const char* token);
TYdbDriverConfig* YdbSetSecureConnection(TYdbDriverConfig* config, const char* cert);

// Получение результата конфигурации
EYdbDriverConfigStatus YdbGetDriverConfigStatus(TYdbDriverConfig* config);
const char* YdbGetDriverConfigErrorMessage(TYdbDriverConfig* config);

// Создание и уничтожение драйвера
TYdbDriver* YdbCreateDriver(const char* connectionString);
TYdbDriver* YdbCreateDriverFromConfig(TYdbDriverConfig* config);
void YdbDestroyDriver(TYdbDriver* driver);

// Получение результата драйвера
EYdbDriverStatus YdbGetDriverStatus(TYdbDriver* driver);
const char* YdbGetDriverErrorMessage(TYdbDriver* driver);

#ifdef __cplusplus
}
#endif
