add_library(impl-ydb_internal-session_pool)

target_link_libraries(impl-ydb_internal-session_pool PUBLIC
  yutil
  cpp-threading-future
  api-protos
  client-impl-ydb_endpoints
  client-ydb_types-operation
  public-issue-protos
)

target_sources(impl-ydb_internal-session_pool PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/client/impl/ydb_internal/session_pool/CMakeLists.txt
  session_pool.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_internal/session_pool/session_pool.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/impl/ydb_internal/session_pool/CMakeLists.txt
=======
  session_pool.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/impl/ydb_internal/session_pool/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  session_pool.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  session_pool.cpp
>>>>>>> origin/main
)
