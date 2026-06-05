find_package(userver REQUIRED COMPONENTS core)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/UserverYdbFromTree.cmake)
userver_ydb_from_tree()

add_subdirectory(key_value)
