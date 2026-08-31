find_package(userver REQUIRED COMPONENTS core)
# userver-ydb links Query in addition to the components used directly by the
# workloads. Keep this request in the staged harness so baseline builds use it
# as well as the current checkout.
if(NOT TARGET YDB-CPP-SDK::Query)
  find_package(ydb-cpp-sdk REQUIRED COMPONENTS Query)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmake/UserverYdbFromTree.cmake)
userver_ydb_from_tree()

add_subdirectory(key_value)
add_subdirectory(topic)
