_ydb_sdk_add_library(topic_it_utils)

target_sources(topic_it_utils
  PRIVATE
    trace.cpp
)

target_link_libraries(topic_it_utils
  PUBLIC
    string_utils-helpers
    YDB-CPP-SDK::Topic
)
