add_ydb_test(NAME dbg_output-dbg_output_ut
  SOURCES
    dbg_output_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/dbg_output
    ${YDB_SDK_SOURCE_DIR}/src/library/testing/unittest
  LINK_LIBRARIES
    yutil
    dbg_output
    cpp-testing-unittest_main
  LABELS
    unit
)
