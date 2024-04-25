add_library(library-cpp-colorizer)

target_link_libraries(library-cpp-colorizer PUBLIC
  yutil
)

target_sources(library-cpp-colorizer PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/colorizer/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/colorizer/colors.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/colorizer/output.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/colorizer/colors.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/colorizer/output.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/colorizer/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/colorizer/colors.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/colorizer/output.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/colorizer/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/colorizer/colors.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/colorizer/output.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/colorizer/colors.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/colorizer/output.cpp
>>>>>>> origin/main
)
