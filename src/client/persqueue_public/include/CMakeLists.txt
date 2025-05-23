_ydb_sdk_add_library(client-ydb_persqueue_public-include)

target_link_libraries(client-ydb_persqueue_public-include PUBLIC
  yutil
  api-grpc
  api-grpc-draft
  api-protos
)

generate_enum_serilization(client-ydb_persqueue_public-include
  ${YDB_SDK_SOURCE_DIR}/src/client/persqueue_public/include/control_plane.h
  INCLUDE_HEADERS
  src/client/persqueue_public/include/control_plane.h
)

generate_enum_serilization(client-ydb_persqueue_public-include
  ${YDB_SDK_SOURCE_DIR}/src/client/persqueue_public/include/read_events.h
  INCLUDE_HEADERS
  src/client/persqueue_public/include/read_events.h
)

generate_enum_serilization(client-ydb_persqueue_public-include
  ${YDB_SDK_SOURCE_DIR}/src/client/persqueue_public/include/write_events.h
  INCLUDE_HEADERS
  src/client/persqueue_public/include/write_events.h
)

generate_enum_serilization(client-ydb_persqueue_public-include
  ${YDB_SDK_SOURCE_DIR}/src/client/persqueue_public/include/write_session.h
  INCLUDE_HEADERS
  src/client/persqueue_public/include/write_session.h
)
