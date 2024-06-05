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

add_ydb_test(NAME util-network-address_ut
  SOURCES
    address_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/util/network
  LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
)

add_ydb_test(NAME util-network-endpoint_ut
  SOURCES
    endpoint_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/util/network
  LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
)

add_ydb_test(NAME util-network-ip_ut
  SOURCES
    ip_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/util/network
  LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
)

add_ydb_test(NAME util-network-poller_ut
  SOURCES
    poller_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/util/network
  LINK_LIBRARIES
    yutil
    cpp-testing-unittest_main
)

add_ydb_test(NAME util-network-sock_ut
  SOURCES
    sock_ut.cpp
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}/src/util/network
  LINK_LIBRARIES
    yutil
    cpp-threading-future
    cpp-testing-unittest_main
)
