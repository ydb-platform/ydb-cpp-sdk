add_library(cpp-string_utils-relaxed_escaper)

target_link_libraries(cpp-string_utils-relaxed_escaper PUBLIC
  
  yutil
)

target_sources(cpp-string_utils-relaxed_escaper PRIVATE
<<<<<<< HEAD:src/library/string_utils/relaxed_escaper/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/string_utils/relaxed_escaper/relaxed_escaper.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/string_utils/relaxed_escaper/relaxed_escaper.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/string_utils/relaxed_escaper/CMakeLists.txt
)


