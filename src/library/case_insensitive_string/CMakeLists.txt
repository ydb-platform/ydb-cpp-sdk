add_library(library-cpp-case_insensitive_string)

target_link_libraries(library-cpp-case_insensitive_string PUBLIC
  yutil
  contrib-libs-libc_compat
  cpp-digest-murmur
)

target_sources(library-cpp-case_insensitive_string PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/case_insensitive_string/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/case_insensitive_string/case_insensitive_char_traits.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/case_insensitive_string/case_insensitive_string.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/case_insensitive_string/case_insensitive_char_traits.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/case_insensitive_string/case_insensitive_string.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/case_insensitive_string/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/case_insensitive_string/case_insensitive_char_traits.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/case_insensitive_string/case_insensitive_string.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/case_insensitive_string/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/case_insensitive_string/case_insensitive_char_traits.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/case_insensitive_string/case_insensitive_string.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/case_insensitive_string/case_insensitive_char_traits.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/case_insensitive_string/case_insensitive_string.cpp
>>>>>>> origin/main
)
