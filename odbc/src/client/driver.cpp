#include "client/driver.h"

#include <ydb-cpp-sdk/client/driver/driver.h>

extern "C" {

void* YDB_CreateDriver(const char* endpoint, const char* user, const char* password) {
    try {
        auto config = NYdb::TDriverConfig().SetEndpoint(std::string(endpoint, strlen(endpoint)));

        auto* driver = new NYdb::TDriver(config);
        return static_cast<void*>(driver);
    } catch (...) {
        return nullptr;
    }
}

void YDB_DestroyDriver(void* driver) {
    if (driver) {
        auto* ydb_driver = static_cast<NYdb::TDriver*>(driver);
        delete ydb_driver;
    }
}

}
