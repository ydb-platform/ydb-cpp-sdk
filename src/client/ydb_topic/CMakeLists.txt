add_subdirectory(codecs)
add_subdirectory(impl)

add_library(cpp-client-ydb_topic)

target_link_libraries(cpp-client-ydb_topic PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
  client-ydb_topic-codecs
  library-cpp-retry
  client-ydb_topic-impl
  cpp-client-ydb_proto
  cpp-client-ydb_driver
  api-grpc
  api-grpc-draft
  api-protos
  cpp-client-ydb_table
)

target_sources(cpp-client-ydb_topic PRIVATE
<<<<<<<< HEAD:src/client/topic/CMakeLists.txt
<<<<<<< HEAD:src/client/topic/CMakeLists.txt
  proto_accessor.cpp
)

generate_enum_serilization(cpp-client-ydb_topic
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/client/topic/topic.h
=======
  ${YDB_SDK_SOURCE_DIR}/client/ydb_topic/proto_accessor.cpp
)

generate_enum_serilization(cpp-client-ydb_topic
  ${YDB_SDK_SOURCE_DIR}/client/ydb_topic/topic.h
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_topic/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/ydb_topic/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/client/topic/topic.h
========
  proto_accessor.cpp
)

generate_enum_serilization(cpp-client-ydb_topic
  ${YDB_SDK_SOURCE_DIR}/src/client/ydb_topic/topic.h
  INCLUDE_HEADERS
  src/client/ydb_topic/topic.h
<<<<<<< HEAD
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_topic/CMakeLists.txt
=======
>>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149)):src/client/ydb_topic/CMakeLists.txt
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)

add_library(YDB-CPP-SDK::Topic ALIAS cpp-client-ydb_topic)
