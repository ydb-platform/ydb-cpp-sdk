add_subdirectory(lite)

add_library(library-cpp-charset)

target_link_libraries(library-cpp-charset PUBLIC
  yutil
  cpp-charset-lite
  Iconv::Iconv
)

target_sources(library-cpp-charset PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/charset/decodeunknownplane.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/charset/wide.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/charset/iconv.cpp
)
