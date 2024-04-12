add_library(cpp-yt-misc)

target_compile_options(cpp-yt-misc PRIVATE
  -Wdeprecated-this-capture
)

target_link_libraries(cpp-yt-misc PUBLIC
  yutil
  cpp-yt-exception
  tools-enum_parser-enum_serialization_runtime
  cpp-string_utils-misc
)

target_sources(cpp-yt-misc PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD:src/library/yt/misc/CMakeLists.txt
=======
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
<<<<<<< HEAD:src/library/yt/misc/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
<<<<<<< HEAD:src/library/yt/misc/CMakeLists.txt
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/misc/guid.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/misc/source_location.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/misc/thread_name.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/misc/variant.cpp
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/misc/guid.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/misc/source_location.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/misc/thread_name.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/misc/variant.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/yt/misc/CMakeLists.txt
=======
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/yt/misc/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)


