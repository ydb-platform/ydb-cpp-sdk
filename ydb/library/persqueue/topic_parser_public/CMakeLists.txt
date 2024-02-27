add_library(library-persqueue-topic_parser_public)

target_link_libraries(library-persqueue-topic_parser_public PUBLIC
  contrib-libs-cxxsupp
  yutil
  cpp-string_builder
)

target_sources(library-persqueue-topic_parser_public PRIVATE
  ${CMAKE_SOURCE_DIR}/ydb/library/persqueue/topic_parser_public/topic_parser.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(library-persqueue-topic_parser_public PUBLIC
    contrib-libs-linux-headers
  )
endif()
