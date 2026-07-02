# Override userver's SetupYdbCppSDK.cmake: use already configured
# YDB-CPP-SDK targets instead of downloading ydb-cpp-sdk via CPM.

if(NOT TARGET YDB-CPP-SDK::Table)
    message(FATAL_ERROR "YDB-CPP-SDK::Table must be configured before building userver-ydb")
endif()

set(ydb-cpp-sdk_INCLUDE_DIRS "")
