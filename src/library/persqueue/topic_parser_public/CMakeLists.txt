add_library(library-persqueue-topic_parser_public)

target_link_libraries(library-persqueue-topic_parser_public PUBLIC
  yutil
)

target_sources(library-persqueue-topic_parser_public PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/persqueue/topic_parser_public/CMakeLists.txt
  topic_parser.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/ydb/library/persqueue/topic_parser_public/topic_parser.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/library/persqueue/topic_parser_public/CMakeLists.txt
=======
  topic_parser.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/library/persqueue/topic_parser_public/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  topic_parser.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)
