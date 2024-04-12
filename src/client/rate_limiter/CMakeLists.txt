add_library(cpp-client-ydb_rate_limiter)

target_link_libraries(cpp-client-ydb_rate_limiter PUBLIC
  yutil
  api-grpc
  client-ydb_common_client-impl
  cpp-client-ydb_driver
)

target_sources(cpp-client-ydb_rate_limiter PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<<< HEAD:src/client/rate_limiter/CMakeLists.txt
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
<<<<<<<< HEAD:src/client/rate_limiter/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/client/rate_limiter/CMakeLists.txt
  rate_limiter.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_rate_limiter/rate_limiter.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_rate_limiter/CMakeLists.txt
========
  rate_limiter.cpp
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_rate_limiter/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_rate_limiter/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
========
  rate_limiter.cpp
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_rate_limiter/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)

add_library(YDB-CPP-SDK::RateLimiter ALIAS cpp-client-ydb_rate_limiter)
