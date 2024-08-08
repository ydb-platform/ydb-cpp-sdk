add_ydb_test(NAME getopt-last_getopt_ut
  SOURCES
    last_getopt_ut.cpp
  LINK_LIBRARIES
    getopt
    cpp-testing-unittest_main
  LABELS
    unit
)

add_ydb_test(NAME getopt-modchooser_ut
  SOURCES
    modchooser_ut.cpp
  LINK_LIBRARIES
    getopt
    cpp-testing-unittest_main
  LABELS
    unit
)

add_ydb_test(NAME getopt-opt2_ut
  SOURCES
    opt2_ut.cpp
  LINK_LIBRARIES
    getopt
    cpp-testing-unittest_main
  LABELS
    unit
)

add_ydb_test(NAME getopt-opt_ut
  SOURCES
    opt_ut.cpp
  LINK_LIBRARIES
    getopt
    cpp-testing-unittest_main
  LABELS
    unit
)

add_ydb_test(NAME getopt-posix_getopt_ut
  SOURCES
    posix_getopt_ut.cpp
  LINK_LIBRARIES
    getopt
    cpp-testing-unittest_main
  LABELS
    unit
)

add_ydb_test(NAME getopt-wrap_ut
  SOURCES
    wrap.cpp
  LINK_LIBRARIES
    getopt-small
    cpp-testing-unittest_main
  LABELS
    unit
)

add_ydb_test(NAME getopt-ygetopt_ut
  SOURCES
    ygetopt_ut.cpp
  LINK_LIBRARIES
    getopt
    cpp-testing-unittest_main
  LABELS
    unit
)
