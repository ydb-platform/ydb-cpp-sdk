add_library(ydb-library-uuid)

target_link_libraries(ydb-library-uuid PUBLIC
  yutil
)

target_sources(ydb-library-uuid PRIVATE
  ${CMAKE_SOURCE_DIR}/ydb/library/uuid/uuid.cpp
)
