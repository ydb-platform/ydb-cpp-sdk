add_library(library-persqueue-topic_parser_public)

target_link_libraries(library-persqueue-topic_parser_public PUBLIC
  yutil
)

target_sources(library-persqueue-topic_parser_public PRIVATE
<<<<<<< HEAD:src/library/persqueue/topic_parser_public/CMakeLists.txt
  topic_parser.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/ydb/library/persqueue/topic_parser_public/topic_parser.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/library/persqueue/topic_parser_public/CMakeLists.txt
)
