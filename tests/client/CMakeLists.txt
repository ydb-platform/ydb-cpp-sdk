add_ydb_test(NAME client-ydb_coordination-ut
<<<<<<< HEAD
<<<<<<< HEAD
=======
  INCLUDE_DIRS
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
  ${YDB_SDK_SOURCE_DIR}/client/ydb_coordination
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_coordination
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_coordination
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135))
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_coordination
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_coordination
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main
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
<<<<<<< HEAD
=======
  INCLUDE_DIRS
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
  ${YDB_SDK_SOURCE_DIR}/client/extensions/discovery_mutator
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/extensions/discovery_mutator
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/client/extensions/discovery_mutator
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135))
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/client/extensions/discovery_mutator
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/extensions/discovery_mutator
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main
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
<<<<<<< HEAD
=======
  INCLUDE_DIRS
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
  ${YDB_SDK_SOURCE_DIR}/client/ydb_driver
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_driver
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_driver
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135))
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_driver
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_driver
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main
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
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
  ${YDB_SDK_SOURCE_DIR}/src/client/impl/ydb_endpoints
=======
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_endpoints
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/impl/ydb_endpoints
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
  ${YDB_SDK_SOURCE_DIR}/src/client/impl/ydb_endpoints
=======
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_endpoints
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/impl/ydb_endpoints
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/impl/ydb_endpoints
>>>>>>> origin/main
  SOURCES
  endpoints_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  client-impl-ydb_endpoints
)

add_ydb_test(NAME client-ydb_params-ut
<<<<<<< HEAD
<<<<<<< HEAD
=======
  INCLUDE_DIRS
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
  ${YDB_SDK_SOURCE_DIR}/client/ydb_params
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_params
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_params
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135))
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_params
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_params
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main
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
<<<<<<< HEAD
=======
  INCLUDE_DIRS
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
  ${YDB_SDK_SOURCE_DIR}/client/ydb_result
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_result
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_result
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135))
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_result
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_result
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main
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
<<<<<<< HEAD
=======
  INCLUDE_DIRS
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
  ${YDB_SDK_SOURCE_DIR}/client/ydb_value
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_value
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_value
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135))
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_value
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_value
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main
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
<<<<<<< HEAD
=======
  INCLUDE_DIRS
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
  ${YDB_SDK_SOURCE_DIR}/client/draft
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/draft
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/client/draft
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135))
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/client/draft
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135))
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/draft
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main
  SOURCES
  ydb_scripting_response_headers_ut.cpp
  LINK_LIBRARIES
  yutil
  cpp-testing-unittest_main
  YDB-CPP-SDK::Draft
)
