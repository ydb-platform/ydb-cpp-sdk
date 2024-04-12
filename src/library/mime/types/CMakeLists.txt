add_library(cpp-mime-types)

target_link_libraries(cpp-mime-types PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
)

target_sources(cpp-mime-types PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/mime/types/mime.cpp
)

generate_enum_serilization(cpp-mime-types
  ${YDB_SDK_SOURCE_DIR}/src/library/mime/types/mime.h
  INCLUDE_HEADERS
  src/library/mime/types/mime.h
)
