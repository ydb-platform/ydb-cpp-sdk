add_ydb_test(NAME util-digest-city_ut
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/util/digest
  SOURCES
    city_ut.cpp
  LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
)

add_ydb_test(NAME util-digest-fnv_ut
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/util/digest
  SOURCES
    fnv_ut.cpp
  LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
)

add_ydb_test(NAME util-digest-multi_ut
  SOURCES
    multi_ut.cpp
  LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
)

add_ydb_test(NAME util-digest-murmur_ut
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/util/digest
  SOURCES
    murmur_ut.cpp
  LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
)

add_ydb_test(NAME util-digest-sequence_ut
  SOURCES
    sequence_ut.cpp
  LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
)
