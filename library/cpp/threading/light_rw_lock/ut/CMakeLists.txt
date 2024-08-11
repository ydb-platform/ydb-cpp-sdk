add_ydb_test(NAME threading-light_rw_lock-ut
  SOURCES
    rwlock_ut.cpp
  LINK_LIBRARIES
    threading-light_rw_lock
    cpp-testing-unittest_main
  LABELS
    unit
)
