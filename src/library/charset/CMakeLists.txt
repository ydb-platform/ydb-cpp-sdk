add_subdirectory(lite)

add_library(library-cpp-charset)

target_link_libraries(library-cpp-charset PUBLIC
  yutil
  cpp-charset-lite
  Iconv::Iconv
)

target_sources(library-cpp-charset PRIVATE
<<<<<<< HEAD:src/library/charset/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/charset/decodeunknownplane.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/charset/wide.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/charset/iconv.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/charset/decodeunknownplane.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/charset/wide.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/charset/iconv.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/charset/CMakeLists.txt
)
