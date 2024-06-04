add_ydb_test(NAME library-json-writer-json_ut
SOURCES
    json_ut.cpp
LINK_LIBRARIES
    yutil
    cpp-json-writer
    cpp-testing-unittest_main
)

add_ydb_test(NAME library-json-writer-json_value_ut
SOURCES
    json_value_ut.cpp
LINK_LIBRARIES
    yutil
    cpp-json-writer
    cpp-testing-unittest_main
)