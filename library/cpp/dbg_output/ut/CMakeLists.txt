add_ydb_test(NAME dbg_output-dbg_output_ut
  SOURCES
    dbg_output_ut.cpp
  LINK_LIBRARIES
    yutil
    dbg_output
    cpp-testing-unittest_main
  LABELS
    unit
)
