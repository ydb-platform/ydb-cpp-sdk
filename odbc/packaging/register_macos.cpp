#include <odbcinst.h>

#include <array>
#include <cstdio>

namespace {

struct TProfileEntry {
    const char* File;
    const char* Section;
    const char* Key;
    const char* Value;
};

void PrintInstallerErrors() {
    for (WORD record = 1;; ++record) {
        DWORD errorCode = 0;
        char message[1024] = {};
        WORD messageLength = 0;
        const SQLRETURN result = SQLInstallerError(
            record, &errorCode, message, sizeof(message), &messageLength);
        if (result == SQL_NO_DATA) {
            return;
        }
        if (result != SQL_SUCCESS && result != SQL_SUCCESS_WITH_INFO) {
            return;
        }
        std::fprintf(stderr, "iODBC installer error %lu: %.*s\n",
                     static_cast<unsigned long>(errorCode),
                     static_cast<int>(messageLength), message);
    }
}

bool WriteEntry(const TProfileEntry& entry) {
    if (SQLWritePrivateProfileString(
            entry.Section, entry.Key, entry.Value, entry.File)) {
        return true;
    }

    std::fprintf(stderr, "Failed to write [%s] %s to %s\n",
                 entry.Section, entry.Key, entry.File);
    PrintInstallerErrors();
    return false;
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::fprintf(stderr, "Usage: %s /absolute/path/to/libydb-odbc.dylib\n", argv[0]);
        return 2;
    }

    if (!SQLSetConfigMode(ODBC_SYSTEM_DSN)) {
        std::fputs("Failed to select the system iODBC configuration\n", stderr);
        PrintInstallerErrors();
        return 1;
    }

    const char* driverPath = argv[1];
    const std::array entries = {
        TProfileEntry{"odbcinst.ini", "ODBC Drivers", "YDB ODBC Driver", "Installed"},
        TProfileEntry{"odbcinst.ini", "YDB ODBC Driver", "Description", "YDB ODBC Driver"},
        TProfileEntry{"odbcinst.ini", "YDB ODBC Driver", "Driver", driverPath},
        TProfileEntry{"odbcinst.ini", "YDB ODBC Driver", "Setup", driverPath},
        TProfileEntry{"odbcinst.ini", "YDB ODBC Driver", "APILevel", "1"},
        TProfileEntry{"odbcinst.ini", "YDB ODBC Driver", "ConnectFunctions", "YYY"},
        TProfileEntry{"odbcinst.ini", "YDB ODBC Driver", "DriverODBCVer", "03.00"},
        TProfileEntry{"odbcinst.ini", "YDB ODBC Driver", "FileUsage", "0"},
        TProfileEntry{"odbc.ini", "ODBC Data Sources", "YDB", "YDB ODBC Driver"},
        TProfileEntry{"odbc.ini", "YDB", "Driver", driverPath},
        TProfileEntry{"odbc.ini", "YDB", "Description", "Local YDB"},
        TProfileEntry{"odbc.ini", "YDB", "Endpoint", "grpc://localhost:2136"},
        TProfileEntry{"odbc.ini", "YDB", "Database", "/local"},
        TProfileEntry{"odbc.ini", "YDB", "AuthMode", "Anonymous"},
    };

    for (const auto& entry : entries) {
        if (!WriteEntry(entry)) {
            return 1;
        }
    }

    return 0;
}
