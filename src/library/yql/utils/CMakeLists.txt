add_library(library-yql-utils)

target_link_libraries(library-yql-utils PUBLIC
  yutil
  cpp-string_utils-quote
  cpp-threading-future
  cpp-deprecated-atomic
)

target_sources(library-yql-utils PRIVATE
  utf8.cpp
)
