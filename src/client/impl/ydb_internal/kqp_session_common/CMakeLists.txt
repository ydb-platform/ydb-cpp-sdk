add_library(impl-ydb_internal-kqp_session_common)

target_link_libraries(impl-ydb_internal-kqp_session_common PUBLIC
  yutil
  cpp-threading-future
  lib-operation_id-protos
  client-impl-ydb_endpoints
)

target_sources(impl-ydb_internal-kqp_session_common PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/client/impl/ydb_internal/kqp_session_common/CMakeLists.txt
  kqp_session_common.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_internal/kqp_session_common/kqp_session_common.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/impl/ydb_internal/kqp_session_common/CMakeLists.txt
=======
  kqp_session_common.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)


