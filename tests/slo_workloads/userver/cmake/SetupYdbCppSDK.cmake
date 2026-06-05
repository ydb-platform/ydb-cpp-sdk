# Override userver's SetupYdbCppSDK.cmake: use in-tree YDB-CPP-SDK targets
# from this repo instead of downloading ydb-cpp-sdk via CPM.

if(NOT TARGET YDB-CPP-SDK::Table)
    message(FATAL_ERROR "In-tree YDB-CPP-SDK must be configured before building userver-ydb")
endif()

set(ydb-cpp-sdk_INCLUDE_DIRS "")
