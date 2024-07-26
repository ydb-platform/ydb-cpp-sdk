add_subdirectory(client)
add_subdirectory(json_value)
add_subdirectory(library)
add_subdirectory(util)

add_executable(some_code)

target_sources(some_code PRIVATE
  some_code.cpp
)

target_link_libraries(some_code PRIVATE
  yutil
)
