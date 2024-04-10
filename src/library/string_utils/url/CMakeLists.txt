add_library(cpp-string_utils-url)

target_link_libraries(cpp-string_utils-url PUBLIC
  yutil
  cpp-string_utils-misc
)

target_sources(cpp-string_utils-url PRIVATE
<<<<<<< HEAD:src/library/string_utils/url/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/string_utils/url/url.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/string_utils/url/url.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/string_utils/url/CMakeLists.txt
)


