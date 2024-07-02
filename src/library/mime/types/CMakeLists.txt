add_library(library-mime-types)

target_link_libraries(library-mime-types PUBLIC
  yutil
  enum_serialization_runtime
)

target_sources(library-mime-types PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/mime/types/mime.cpp
)

generate_enum_serilization(library-mime-types
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/library/mime/types/mime.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/library/mime/types/mime.h
)

_ydb_sdk_install_targets(TARGETS library-mime-types)
