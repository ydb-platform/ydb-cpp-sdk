add_subdirectory(protos)
add_subdirectory(ut)

add_library(public-lib-operation_id)

target_link_libraries(public-lib-operation_id PUBLIC
  yutil
  protobuf::libprotobuf
  library-cpp-cgiparam
  library-cpp-uri
  lib-operation_id-protos
)

target_sources(public-lib-operation_id PRIVATE
  ${YDB_SDK_SOURCE_DIR}/ydb/public/lib/operation_id/operation_id.cpp
)


