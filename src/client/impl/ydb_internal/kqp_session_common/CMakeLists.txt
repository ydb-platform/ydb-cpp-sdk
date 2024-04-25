add_library(impl-ydb_internal-kqp_session_common)

target_link_libraries(impl-ydb_internal-kqp_session_common PUBLIC
  yutil
  cpp-threading-future
  lib-operation_id-protos
  client-impl-ydb_endpoints
)

target_sources(impl-ydb_internal-kqp_session_common PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/client/impl/ydb_internal/kqp_session_common/CMakeLists.txt
  kqp_session_common.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_internal/kqp_session_common/kqp_session_common.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/impl/ydb_internal/kqp_session_common/CMakeLists.txt
=======
  kqp_session_common.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/impl/ydb_internal/kqp_session_common/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  kqp_session_common.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  kqp_session_common.cpp
>>>>>>> origin/main
)


