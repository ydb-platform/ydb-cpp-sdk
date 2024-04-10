add_subdirectory(lite)

add_library(library-cpp-charset)

target_link_libraries(library-cpp-charset PUBLIC
  yutil
  cpp-charset-lite
  Iconv::Iconv
)

target_sources(library-cpp-charset PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/library/charset/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/charset/decodeunknownplane.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/charset/wide.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/charset/iconv.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/charset/decodeunknownplane.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/charset/wide.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/charset/iconv.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/charset/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/charset/decodeunknownplane.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/charset/wide.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/charset/iconv.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/charset/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)
