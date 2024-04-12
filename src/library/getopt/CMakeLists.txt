add_subdirectory(small)

add_library(library-cpp-getopt INTERFACE)
target_link_libraries(library-cpp-getopt INTERFACE
  yutil
  cpp-getopt-small
  library-cpp-svnversion
  library-cpp-build_info
)

add_global_library_for(library-cpp-getopt.global library-cpp-getopt)
target_link_libraries(library-cpp-getopt.global PUBLIC
  yutil
  cpp-getopt-small
  library-cpp-svnversion
  library-cpp-build_info
)
target_sources(library-cpp-getopt.global PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/library/getopt/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/getopt/print.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/getopt/print.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/getopt/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/getopt/print.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(library-cpp-getopt INTERFACE
    
  )
  target_link_libraries(library-cpp-getopt.global PUBLIC
    
  )
endif()
