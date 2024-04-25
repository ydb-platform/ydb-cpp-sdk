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
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/yt/yson_string/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/yson_string/convert.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/yson_string/string.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/yson_string/convert.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/yson_string/string.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/yt/yson_string/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/yson_string/convert.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/yson_string/string.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/yt/yson_string/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/yson_string/convert.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/yson_string/string.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/yson_string/convert.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/yson_string/string.cpp
>>>>>>> origin/main
)

if (NOT (WIN32 AND CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64"))
  target_compile_options(cpp-yt-yson_string PRIVATE
    -Wdeprecated-this-capture
  )
endif ()
