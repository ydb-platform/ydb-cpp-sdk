add_library(cpp-yt-assert)

target_compile_options(cpp-yt-assert PRIVATE
  -Wdeprecated-this-capture
)

target_link_libraries(cpp-yt-assert PUBLIC
  
  yutil
)

target_sources(cpp-yt-assert PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/library/yt/assert/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/assert/assert.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/assert/assert.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/yt/assert/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/assert/assert.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)


