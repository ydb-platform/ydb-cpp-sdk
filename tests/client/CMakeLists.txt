add_ydb_test(NAME client-ydb_coordination-ut
<<<<<<< HEAD
=======
  INCLUDE_DIRS
<<<<<<< HEAD
  ${YDB_SDK_SOURCE_DIR}/client/ydb_coordination
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_coordination
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
  SOURCES
  coordination_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  YDB-CPP-SDK::Coordination
  api-grpc
)

add_ydb_test(NAME client-extensions-discovery_mutator-ut
<<<<<<< HEAD
=======
  INCLUDE_DIRS
<<<<<<< HEAD
  ${YDB_SDK_SOURCE_DIR}/client/extensions/discovery_mutator
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/extensions/discovery_mutator
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
  SOURCES
  discovery_mutator_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  YDB-CPP-SDK::DiscoveryMutator
  YDB-CPP-SDK::Table
)

add_ydb_test(NAME client-ydb_driver-ut
<<<<<<< HEAD
=======
  INCLUDE_DIRS
<<<<<<< HEAD
  ${YDB_SDK_SOURCE_DIR}/client/ydb_driver
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_driver
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
  SOURCES
  driver_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  YDB-CPP-SDK::Driver
  YDB-CPP-SDK::Table
)

add_ydb_test(NAME client-impl-ydb_endpoints-ut
  INCLUDE_DIRS
<<<<<<< HEAD
<<<<<<< HEAD
  ${YDB_SDK_SOURCE_DIR}/src/client/impl/ydb_endpoints
=======
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_endpoints
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/impl/ydb_endpoints
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
  SOURCES
  endpoints_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  client-impl-ydb_endpoints
)

add_ydb_test(NAME client-ydb_params-ut
<<<<<<< HEAD
=======
  INCLUDE_DIRS
<<<<<<< HEAD
  ${YDB_SDK_SOURCE_DIR}/client/ydb_params
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_params
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
  SOURCES
  params_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  YDB-CPP-SDK::Params
  YDB-CPP-SDK::YsonValue
)

add_ydb_test(NAME client-ydb_result-ut
<<<<<<< HEAD
=======
  INCLUDE_DIRS
<<<<<<< HEAD
  ${YDB_SDK_SOURCE_DIR}/client/ydb_result
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_result
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
  SOURCES
  result_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  YDB-CPP-SDK::Result
  YDB-CPP-SDK::Params
)

add_ydb_test(NAME client-ydb_value-ut
<<<<<<< HEAD
=======
  INCLUDE_DIRS
<<<<<<< HEAD
  ${YDB_SDK_SOURCE_DIR}/client/ydb_value
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_value
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
  SOURCES
  value_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  YDB-CPP-SDK::Value
  YDB-CPP-SDK::JsonValue
  YDB-CPP-SDK::YsonValue
  YDB-CPP-SDK::Params
)

add_ydb_test(NAME client-draft-ut
<<<<<<< HEAD
=======
  INCLUDE_DIRS
<<<<<<< HEAD
  ${YDB_SDK_SOURCE_DIR}/client/draft
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/draft
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
  SOURCES
  ydb_scripting_response_headers_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  YDB-CPP-SDK::Draft
)
