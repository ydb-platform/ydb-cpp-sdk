add_library(library-cpp-svnversion)

target_link_libraries(library-cpp-svnversion PUBLIC
  yutil
  cpp-string_utils-misc
)

target_sources(library-cpp-svnversion PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/svnversion/svnversion.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/svnversion/svn_interface.c
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(library-cpp-svnversion PUBLIC
    
  )
endif()
