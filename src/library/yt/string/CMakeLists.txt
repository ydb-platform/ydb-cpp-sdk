add_library(cpp-yt-string)

target_compile_options(cpp-yt-string PRIVATE
  -Wdeprecated-this-capture
)

target_link_libraries(cpp-yt-string PUBLIC
  
  yutil
  cpp-yt-assert
  cpp-yt-exception
  cpp-yt-misc
  cpp-yt-small_containers
  cpp-string_utils-misc
)

target_sources(cpp-yt-string PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD:src/library/yt/string/CMakeLists.txt
=======
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
<<<<<<< HEAD:src/library/yt/string/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
<<<<<<< HEAD:src/library/yt/string/CMakeLists.txt
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/string/enum.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/string/guid.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/string/string.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/string/format.cpp
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/string/enum.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/string/guid.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/string/string.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/string/format.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/yt/string/CMakeLists.txt
=======
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/yt/string/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main
)


