add_ydb_test(NAME util-network-iovec_ut
  SOURCES
    iovec_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/util/network
  LINK_LIBRARIES
    yutil
    cpp-testing-gtest_main
)

add_ydb_test(NAME util-network-socket_ut
  SOURCES
    socket_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/util/network
  LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
)
