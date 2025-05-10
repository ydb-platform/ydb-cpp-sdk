#include <ydb-cpp-sdk/c_api/driver.h>

#include "impl/driver_impl.h" // NOLINT

#include <string>

extern "C" {

// Создание и уничтожение конфигурации
TYdbDriverConfig* YdbCreateDriverConfig(const char* connectionString) {
    try {
        if (!connectionString) {
            return new TYdbDriverConfig{YDB_DRIVER_CONFIG_INVALID, "Invalid config pointer"};
        }

        try {
            return new TYdbDriverConfig{YDB_DRIVER_CONFIG_OK, "", NYdb::TDriverConfig(connectionString)};
        } catch (const std::exception& e) {
            return new TYdbDriverConfig{YDB_DRIVER_CONFIG_INVALID, e.what()};
        }
    } catch (const std::exception& e) {
        return nullptr;
    }
}

void YdbDestroyDriverConfig(TYdbDriverConfig* config) {
    if (config) {
        delete config;
    }
}

// Установка параметров конфигурации
TYdbDriverConfig* YdbSetEndpoint(TYdbDriverConfig* config, const char* endpoint) {
    try {
        if (!config) {
            return new TYdbDriverConfig{YDB_DRIVER_CONFIG_INVALID, "Invalid config"};
        }
        if (!endpoint) {
            return new TYdbDriverConfig{YDB_DRIVER_CONFIG_INVALID, "Invalid endpoint"};
        }

        try {
            config->config.SetEndpoint(std::string(endpoint));
            return config;
        } catch (const std::exception& e) {
            return new TYdbDriverConfig{YDB_DRIVER_CONFIG_INVALID, e.what()};
        }
    } catch (...) {
        return nullptr;
    }
}

TYdbDriverConfig* YdbSetDatabase(TYdbDriverConfig* config, const char* database) {
    try {
        if (!config) {
            return new TYdbDriverConfig{YDB_DRIVER_CONFIG_INVALID, "Invalid config"};
        }
        if (!database) {
            return new TYdbDriverConfig{YDB_DRIVER_CONFIG_INVALID, "Invalid database"};
        }

        try {
            config->config.SetDatabase(std::string(database));
            return config;
        } catch (const std::exception& e) {
            return new TYdbDriverConfig{YDB_DRIVER_CONFIG_INVALID, e.what()};
        }
    } catch (...) {
        return nullptr;
    }
}

TYdbDriverConfig* YdbSetAuthToken(TYdbDriverConfig* config, const char* token) {
    try {
        if (!config) {
            return new TYdbDriverConfig{YDB_DRIVER_CONFIG_INVALID, "Invalid config"};
        }
        if (!token) {
            return new TYdbDriverConfig{YDB_DRIVER_CONFIG_INVALID, "Invalid token"};
        }

        try {
            config->config.SetAuthToken(std::string(token));
            return config;
        } catch (const std::exception& e) {
            return new TYdbDriverConfig{YDB_DRIVER_CONFIG_INVALID, e.what()};
        }
    } catch (...) {
        return nullptr;
    }
}

TYdbDriverConfig* YdbSetSecureConnection(TYdbDriverConfig* config, const char* cert) {
    try {
        if (!config) {
            return new TYdbDriverConfig{YDB_DRIVER_CONFIG_INVALID, "Invalid config"};
        }
        if (!cert) {
            return new TYdbDriverConfig{YDB_DRIVER_CONFIG_INVALID, "Invalid certificate"};
        }

        try {
            config->config.UseSecureConnection(std::string(cert));
            return config;
        } catch (const std::exception& e) {
            return new TYdbDriverConfig{YDB_DRIVER_CONFIG_INVALID, e.what()};
        }
    } catch (...) {
        return nullptr;
    }
}

EYdbDriverConfigStatus YdbGetDriverConfigStatus(TYdbDriverConfig* config) {
    if (!config) {
        return YDB_DRIVER_CONFIG_INVALID;
    }

    return config->errorCode;
}

const char* YdbGetDriverConfigErrorMessage(TYdbDriverConfig* config) {
    if (!config) {
        return "Invalid config";
    }

    return config->errorMessage.c_str();
}

// Создание и уничтожение драйвера
TYdbDriver* YdbCreateDriverFromConfig(TYdbDriverConfig* config) {
    try {
        if (!config) {
            return new TYdbDriver{YDB_DRIVER_ERROR, "Invalid config"};
        }

        if (config->errorCode != 0) {
            return new TYdbDriver{YDB_DRIVER_ERROR, "Invalid config: " + config->errorMessage};
        }

        try {
            return new TYdbDriver{YDB_DRIVER_OK, "", NYdb::TDriver(config->config)};
        } catch (const std::exception& e) {
            return new TYdbDriver{YDB_DRIVER_ERROR, e.what()};
        }
    } catch (...) {
        return nullptr;
    }
}

TYdbDriver* YdbCreateDriver(const char* connectionString) {
    try {
        if (!connectionString) {
            return new TYdbDriver{YDB_DRIVER_ERROR, "Invalid connection string"};
        }

        try {
            return new TYdbDriver{YDB_DRIVER_OK, "", NYdb::TDriver(std::string(connectionString))};
        } catch (const std::exception& e) {
            return new TYdbDriver{YDB_DRIVER_ERROR, e.what()};
        }
    } catch (...) {
        return nullptr;
    }
}

void YdbDestroyDriver(TYdbDriver* driver) {
    if (driver) {
        delete driver;
    }
}

}
