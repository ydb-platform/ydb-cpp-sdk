add_library(library-cpp-cgiparam)

target_link_libraries(library-cpp-cgiparam PUBLIC
  yutil
  library-cpp-iterator
  cpp-string_utils-quote
  cpp-string_utils-scan
)

target_sources(library-cpp-cgiparam PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/cgiparam/cgiparam.cpp
)
