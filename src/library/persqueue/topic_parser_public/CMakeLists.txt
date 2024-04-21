add_library(library-persqueue-topic_parser_public)

target_link_libraries(library-persqueue-topic_parser_public PUBLIC
  yutil
)

target_sources(library-persqueue-topic_parser_public PRIVATE
  topic_parser.cpp
)
