add_library(cpp-yt-yson)

target_compile_options(cpp-yt-yson PRIVATE
  -Wdeprecated-this-capture
)

target_link_libraries(cpp-yt-yson PUBLIC
  yutil
  cpp-yt-yson_string
)

target_sources(cpp-yt-yson PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/yt/yson/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/yson/consumer.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/yson/consumer.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/yt/yson/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/yson/consumer.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/yt/yson/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/yson/consumer.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)


