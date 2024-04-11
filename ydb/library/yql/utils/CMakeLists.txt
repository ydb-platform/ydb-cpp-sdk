add_subdirectory(ut)

add_library(library-yql-utils)

target_link_libraries(library-yql-utils PUBLIC
  yutil
  cpp-string_utils-quote
  cpp-threading-future
  cpp-deprecated-atomic
)

target_sources(library-yql-utils PRIVATE
  ${YDB_SDK_SOURCE_DIR}/ydb/library/yql/utils/utf8.cpp
)
