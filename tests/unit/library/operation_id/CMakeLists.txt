add_ydb_test(NAME operation_id_ut
  SOURCES
    operation_id_ut.cpp
  LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
    lib-operation_id-protos
    library-operation_id
    cpp-testing-unittest
  LABELS
    unit
)
