add_executable(ydb_tech_topic main.cpp)
add_dependencies(ydb_tech_examples ydb_tech_topic)
target_link_libraries(ydb_tech_topic PRIVATE
  yutil
  YDB-CPP-SDK::Driver
  YDB-CPP-SDK::Topic
  YDB-CPP-SDK::Query
)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_options(ydb_tech_topic PRIVATE -ldl -lrt -lpthread)
endif()
