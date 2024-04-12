add_library(library-cpp-cppparser)

target_link_libraries(library-cpp-cppparser PUBLIC
  yutil
)

target_sources(library-cpp-cppparser PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/cppparser/parser.cpp
)


