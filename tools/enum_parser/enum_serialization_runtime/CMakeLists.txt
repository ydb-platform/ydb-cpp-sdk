add_library(tools-enum_parser-enum_serialization_runtime)

target_link_libraries(tools-enum_parser-enum_serialization_runtime PUBLIC
  yutil
)

target_sources(tools-enum_parser-enum_serialization_runtime PRIVATE
  ${CMAKE_SOURCE_DIR}/tools/enum_parser/enum_serialization_runtime/dispatch_methods.cpp
  ${CMAKE_SOURCE_DIR}/tools/enum_parser/enum_serialization_runtime/enum_runtime.cpp
  ${CMAKE_SOURCE_DIR}/tools/enum_parser/enum_serialization_runtime/ordered_pairs.cpp
  ${CMAKE_SOURCE_DIR}/tools/enum_parser/enum_serialization_runtime/serialized_enum.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(tools-enum_parser-enum_serialization_runtime PUBLIC
    
  )
endif()
