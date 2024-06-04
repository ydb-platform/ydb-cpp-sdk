add_ydb_test(NAME json-ut-json_prettifier_ut
SOURCES
    json_prettifier_ut.cpp
LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
    library-cpp-json
)

add_ydb_test(NAME json-ut-json_reader_fast_ut
SOURCES
    json_reader_fast_ut.cpp
LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
    library-cpp-json
)

add_ydb_test(NAME json-ut-json_reader_ut
SOURCES
    json_reader_ut.cpp
LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
    library-cpp-json
)

add_ydb_test(NAME json-ut-json_saveload_ut
SOURCES
    json_saveload_ut.cpp
LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
    library-cpp-json
)

add_ydb_test(NAME json-ut-json_writer_ut
SOURCES
    json_writer_ut.cpp
LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
    library-cpp-json
)

