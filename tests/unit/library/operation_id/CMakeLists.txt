add_ydb_test(NAME operation_id_ut GTEST
  SOURCES
    operation_id_ut.cpp
  LINK_LIBRARIES
    yutil
    lib-operation_id-protos
    library-operation_id
  LABELS
    unit
)
