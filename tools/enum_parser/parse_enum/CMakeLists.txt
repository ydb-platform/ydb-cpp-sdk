add_library(tools-enum_parser-parse_enum)

target_link_libraries(tools-enum_parser-parse_enum PUBLIC
  yutil
  library-cpp-cppparser
  cpp-string_utils-misc
)

target_sources(tools-enum_parser-parse_enum PRIVATE
  ${CMAKE_SOURCE_DIR}/tools/enum_parser/parse_enum/parse_enum.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(tools-enum_parser-parse_enum PUBLIC
    
  )
endif()
