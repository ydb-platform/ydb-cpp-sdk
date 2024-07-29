add_ydb_test(NAME library-digest-md5_medium_ut
  SOURCES
    md5_medium_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/digest/md5
    ${YDB_SDK_SOURCE_DIR}/src/library/testing/unittest
  LINK_LIBRARIES
    yutil
    digest-md5
    cpp-testing-unittest_main
  LABELS
    unit
)

add_ydb_test(NAME library-digest-md5_ut
  SOURCES
    md5_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/digest/md5
    ${YDB_SDK_SOURCE_DIR}/src/library/testing/unittest
  LINK_LIBRARIES
    yutil
    digest-md5
    cpp-testing-unittest_main
  LABELS
    unit
)
