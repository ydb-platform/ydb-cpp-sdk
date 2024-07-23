set(ENDPOINT "localhost:2136")
set(DATABASE "/local")
configure_file(config_ydb.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config_ydb.h @ONLY)

add_subdirectory(basic_example_it)
