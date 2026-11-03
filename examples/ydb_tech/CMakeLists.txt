add_custom_target(ydb_tech_examples)

file(GLOB example_directories CONFIGURE_DEPENDS LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/*")
foreach(example_directory IN LISTS example_directories)
  if(IS_DIRECTORY "${example_directory}" AND EXISTS "${example_directory}/CMakeLists.txt")
    add_subdirectory("${example_directory}")
  endif()
endforeach()
