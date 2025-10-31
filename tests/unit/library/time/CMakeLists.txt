add_ydb_test(NAME time_ut GTEST
  SOURCES
    time_ut.cpp
  LINK_LIBRARIES
    ydb-time
  LABELS
    unit
)
