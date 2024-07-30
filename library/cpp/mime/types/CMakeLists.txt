add_library(mime-types)

target_link_libraries(mime-types
  PUBLIC
    yutil
  PRIVATE
    enum_serialization_runtime
)

target_sources(mime-types PRIVATE
  mime.cpp
)

generate_enum_serilization(mime-types
  ${YDB_SDK_SOURCE_DIR}/library/cpp/mime/types/mime.h
  INCLUDE_HEADERS
  library/cpp/mime/types/mime.h
)

_ydb_sdk_install_targets(TARGETS mime-types)
