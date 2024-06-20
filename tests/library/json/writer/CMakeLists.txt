add_ydb_test(NAME library-json-writer-json_ut
SOURCES
    json_ut.cpp
INCLUDE_DIRS   
    ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/library/json
    ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/library/json/writer
    ${YDB_SDK_SOURCE_DIR}/src/library/testing/unittest
    ${YDB_SDK_SOURCE_DIR}/src/util/system
LINK_LIBRARIES
    yutil
    cpp-json-writer
    cpp-testing-unittest_main
)

add_ydb_test(NAME library-json-writer-json_value_ut
SOURCES
    json_value_ut.cpp
INCLUDE_DIRS   
    ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/library/json
    ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/util/stream
    ${YDB_SDK_SOURCE_DIR}/src/library/testing/unittest
LINK_LIBRARIES
    yutil
    cpp-json-writer
    cpp-testing-unittest_main
)
