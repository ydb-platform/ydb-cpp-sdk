add_library(client-ydb_persqueue_public-codecs)

target_link_libraries(client-ydb_persqueue_public-codecs PUBLIC
  yutil
  cpp-streams-zstd
  public-issue-protos
  api-grpc-draft
  api-grpc
  api-protos
)

target_sources(client-ydb_persqueue_public-codecs PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<<< HEAD:src/client/persqueue_public/codecs/CMakeLists.txt
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
<<<<<<<< HEAD:src/client/persqueue_public/codecs/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/client/persqueue_public/codecs/CMakeLists.txt
  codecs.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_persqueue_public/codecs/codecs.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_persqueue_public/codecs/CMakeLists.txt
========
  codecs.cpp
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_persqueue_public/codecs/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_persqueue_public/codecs/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
========
  codecs.cpp
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_persqueue_public/codecs/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  codecs.cpp
>>>>>>> origin/main
)


