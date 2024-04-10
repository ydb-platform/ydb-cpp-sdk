add_library(client-ydb_topic-codecs)

target_link_libraries(client-ydb_topic-codecs PUBLIC
  yutil
  cpp-streams-zstd
  public-issue-protos
  api-grpc-draft
  api-grpc
  api-protos
)

target_sources(client-ydb_topic-codecs PRIVATE
<<<<<<< HEAD
<<<<<<<< HEAD:src/client/topic/codecs/CMakeLists.txt
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/client/topic/codecs/CMakeLists.txt
  codecs.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_topic/codecs/codecs.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_topic/codecs/CMakeLists.txt
========
  codecs.cpp
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_topic/codecs/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_topic/codecs/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)


