<<<<<<< HEAD:tests/unit/library/diff/CMakeLists.txt
add_ydb_test(NAME library-diff_ut
=======
add_ydb_test(NAME diff-ut
>>>>>>> 740dcae8d (Returned library and switched client to ol util and library):tests/library/diff/CMakeLists.txt
  SOURCES
    ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/library/diff
  LINK_LIBRARIES
    diff
    cpp-testing-unittest_main
  LABELS
    unit
)
