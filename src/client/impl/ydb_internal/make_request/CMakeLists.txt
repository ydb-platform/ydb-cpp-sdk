add_library(impl-ydb_internal-make_request)

target_link_libraries(impl-ydb_internal-make_request PUBLIC
  yutil
  protobuf::libprotobuf
  api-protos
)

target_sources(impl-ydb_internal-make_request PRIVATE
<<<<<<< HEAD:src/client/impl/ydb_internal/make_request/CMakeLists.txt
  make.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_internal/make_request/make.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/impl/ydb_internal/make_request/CMakeLists.txt
)
