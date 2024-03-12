add_library(library-yql-utils)

target_link_libraries(library-yql-utils PUBLIC
  yutil
  cpp-digest-md5
  cpp-string_utils-quote
  cpp-threading-future
  cpp-deprecated-atomic
)

target_sources(library-yql-utils PRIVATE
  ${CMAKE_SOURCE_DIR}/ydb/library/yql/utils/utf8.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(library-yql-utils PUBLIC
    
  )
endif()
