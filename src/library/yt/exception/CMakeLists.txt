add_library(cpp-yt-exception)

target_compile_options(cpp-yt-exception PRIVATE
  -Wdeprecated-this-capture
)

target_link_libraries(cpp-yt-exception PUBLIC
  yutil
)

target_sources(cpp-yt-exception PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/library/yt/exception/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/exception/exception.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/exception/exception.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/yt/exception/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/exception/exception.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)


