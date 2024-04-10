add_library(library-cpp-case_insensitive_string)

target_link_libraries(library-cpp-case_insensitive_string PUBLIC
  yutil
  contrib-libs-libc_compat
  cpp-digest-murmur
)

target_sources(library-cpp-case_insensitive_string PRIVATE
<<<<<<< HEAD:src/library/case_insensitive_string/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/case_insensitive_string/case_insensitive_char_traits.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/case_insensitive_string/case_insensitive_string.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/case_insensitive_string/case_insensitive_char_traits.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/case_insensitive_string/case_insensitive_string.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/case_insensitive_string/CMakeLists.txt
)
