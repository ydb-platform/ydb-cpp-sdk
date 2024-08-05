add_ydb_test(NAME json-ut
  SOURCES
    json_prettifier_ut.cpp
    json_reader_fast_ut.cpp
    json_reader_ut.cpp
    json_saveload_ut.cpp
    json_writer_ut.cpp
  LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
    json
  LABELS
    unit
)
