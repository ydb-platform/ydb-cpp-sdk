add_ydb_test(NAME dns-dns_ut
  SOURCES
    dns_ut.cpp
  LINK_LIBRARIES
    dns
    cpp-testing-unittest_main
  LABELS
    unit
)
