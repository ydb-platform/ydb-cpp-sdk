add_subdirectory(writer)

add_ydb_test(NAME json-ut-json_prettifier_ut
SOURCES
    json_prettifier_ut.cpp
INCLUDE_DIRS   
    ${YDB_SDK_SOURCE_DIR}/src/library/json
    ${YDB_SDK_SOURCE_DIR}/src/library/testing/unittest
LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
<<<<<<< HEAD:tests/unit/library/json/CMakeLists.txt
    library-json
LABELS
    unit
=======
    json
>>>>>>> 740dcae8d (Returned library and switched client to ol util and library):tests/library/json/CMakeLists.txt
)

add_ydb_test(NAME json-ut-json_reader_fast_ut
SOURCES
    json_reader_fast_ut.cpp
INCLUDE_DIRS   
    ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/library/json
    ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/util/string    
    ${YDB_SDK_SOURCE_DIR}/src/library/json
    ${YDB_SDK_SOURCE_DIR}/src/library/testing/unittest
    ${YDB_SDK_SOURCE_DIR}/src/library/string_utils/relaxed_escaper
LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
<<<<<<< HEAD:tests/unit/library/json/CMakeLists.txt
    library-json
LABELS
    unit
=======
    json
>>>>>>> 740dcae8d (Returned library and switched client to ol util and library):tests/library/json/CMakeLists.txt
)

add_ydb_test(NAME json-ut-json_reader_ut
SOURCES
    json_reader_ut.cpp
INCLUDE_DIRS   
    ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/library/json
    ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/util/stream
    ${YDB_SDK_SOURCE_DIR}/src/library/json
    ${YDB_SDK_SOURCE_DIR}/src/library/testing/unittest
LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
<<<<<<< HEAD:tests/unit/library/json/CMakeLists.txt
    library-json
LABELS
    unit
=======
    json
>>>>>>> 740dcae8d (Returned library and switched client to ol util and library):tests/library/json/CMakeLists.txt
)

add_ydb_test(NAME json-ut-json_saveload_ut
SOURCES
    json_saveload_ut.cpp
INCLUDE_DIRS   
    ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/library/json
    ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/util
    ${YDB_SDK_SOURCE_DIR}/src/library/testing/unittest
    ${YDB_SDK_SOURCE_DIR}/src/util/stream
LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
<<<<<<< HEAD:tests/unit/library/json/CMakeLists.txt
    library-json
LABELS
    unit
=======
    json
>>>>>>> 740dcae8d (Returned library and switched client to ol util and library):tests/library/json/CMakeLists.txt
)

add_ydb_test(NAME json-ut-json_writer_ut
SOURCES
    json_writer_ut.cpp
INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/util/stream    
    ${YDB_SDK_SOURCE_DIR}/src/library/json
    ${YDB_SDK_SOURCE_DIR}/src/library/testing/unittest
LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
<<<<<<< HEAD:tests/unit/library/json/CMakeLists.txt
    library-json
LABELS
    unit
=======
    json
>>>>>>> 740dcae8d (Returned library and switched client to ol util and library):tests/library/json/CMakeLists.txt
)
