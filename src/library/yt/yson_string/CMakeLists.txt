add_library(cpp-yt-yson_string)

target_link_libraries(cpp-yt-yson_string PUBLIC
  yutil
  cpp-yt-assert
  cpp-yt-coding
  cpp-yt-exception
  cpp-yt-string
  cpp-yt-memory
  cpp-yt-misc
)

target_sources(cpp-yt-yson_string PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/library/yt/yson_string/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/yson_string/convert.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/yson_string/string.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/yson_string/convert.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/yson_string/string.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/yt/yson_string/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/yson_string/convert.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/yson_string/string.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)

if (NOT (WIN32 AND CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64"))
  target_compile_options(cpp-yt-yson_string PRIVATE
    -Wdeprecated-this-capture
  )
endif ()
