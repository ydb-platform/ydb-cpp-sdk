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
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/getopt/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/getopt/print.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/getopt/print.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/getopt/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/getopt/print.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/getopt/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/getopt/print.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/getopt/print.cpp
>>>>>>> origin/main
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(library-cpp-getopt INTERFACE
    
  )
  target_link_libraries(library-cpp-getopt.global PUBLIC
    
  )
endif()
