add_library(library-persqueue-topic_parser_public)

target_link_libraries(library-persqueue-topic_parser_public PUBLIC
  yutil
)

target_sources(library-persqueue-topic_parser_public PRIVATE
  ${YDB_SDK_SOURCE_DIR}/ydb/library/persqueue/topic_parser_public/topic_parser.cpp
)
