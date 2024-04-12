add_library(library-cpp-cpuid_check INTERFACE)

target_link_libraries(library-cpp-cpuid_check INTERFACE
  yutil
)

add_global_library_for(library-cpp-cpuid_check.global library-cpp-cpuid_check)

target_link_libraries(library-cpp-cpuid_check.global PUBLIC
  yutil
)

target_sources(library-cpp-cpuid_check.global PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/cpuid_check/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/cpuid_check/cpu_id_check.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/cpuid_check/cpu_id_check.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/cpuid_check/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/cpuid_check/cpu_id_check.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/cpuid_check/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/cpuid_check/cpu_id_check.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(library-cpp-cpuid_check INTERFACE
    
  )
  target_link_libraries(library-cpp-cpuid_check.global PUBLIC
    
  )
endif()
