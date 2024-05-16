add_library(library-cpp-case_insensitive_string)

target_link_libraries(library-cpp-case_insensitive_string PUBLIC
  yutil
  cpp-digest-murmur
)

target_sources(library-cpp-case_insensitive_string PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/case_insensitive_string/case_insensitive_char_traits.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/case_insensitive_string/case_insensitive_string.cpp
)
