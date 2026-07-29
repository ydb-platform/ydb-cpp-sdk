add_ydb_test(NAME getopt-ut
  SOURCES
    last_getopt_ut.cpp
    modchooser_ut.cpp
    opt2_ut.cpp
    opt_ut.cpp
    posix_getopt_ut.cpp
    wrap.cpp
    ygetopt_ut.cpp
  LINK_LIBRARIES
    getopt
    cpp-testing-unittest_main
  LABELS
    unit
)
