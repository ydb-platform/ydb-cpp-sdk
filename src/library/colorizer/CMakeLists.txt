add_library(library-cpp-colorizer)

target_link_libraries(library-cpp-colorizer PUBLIC
  yutil
)

target_sources(library-cpp-colorizer PRIVATE
<<<<<<< HEAD:src/library/colorizer/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/colorizer/colors.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/colorizer/output.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/colorizer/colors.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/colorizer/output.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/colorizer/CMakeLists.txt
)
