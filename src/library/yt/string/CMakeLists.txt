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
<<<<<<< HEAD:src/library/yt/string/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/string/enum.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/string/guid.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/string/string.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/string/format.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/string/enum.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/string/guid.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/string/string.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/string/format.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/yt/string/CMakeLists.txt
)


