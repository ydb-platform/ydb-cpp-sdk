add_library(cpp-client-ydb_rate_limiter)

target_link_libraries(cpp-client-ydb_rate_limiter PUBLIC
  yutil
  api-grpc
  client-ydb_common_client-impl
  cpp-client-ydb_driver
)

target_sources(cpp-client-ydb_rate_limiter PRIVATE
<<<<<<< HEAD:src/client/rate_limiter/CMakeLists.txt
  rate_limiter.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_rate_limiter/rate_limiter.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_rate_limiter/CMakeLists.txt
)

add_library(YDB-CPP-SDK::RateLimiter ALIAS cpp-client-ydb_rate_limiter)
