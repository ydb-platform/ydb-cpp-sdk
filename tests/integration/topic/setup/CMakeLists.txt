_ydb_sdk_add_library(topic_it_setup)

target_sources(topic_it_setup
  PRIVATE
    fixture.cpp
)

target_link_libraries(topic_it_setup
  PUBLIC
    YDB-CPP-SDK::Discovery
    YDB-CPP-SDK::Topic
    GTest::gtest
)
