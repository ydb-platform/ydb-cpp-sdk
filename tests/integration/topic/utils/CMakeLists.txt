_ydb_sdk_add_library(topic_it_utils)

target_sources(topic_it_utils
  PRIVATE
    describe.cpp
    local_partition.cpp
    managed_executor.cpp
    setup.cpp
    trace.cpp
)

target_link_libraries(topic_it_utils
  PUBLIC
    string_utils-helpers
    YDB-CPP-SDK::Topic
)
