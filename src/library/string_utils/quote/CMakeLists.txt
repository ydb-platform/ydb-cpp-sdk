add_library(cpp-string_utils-quote)

target_link_libraries(cpp-string_utils-quote PUBLIC
  
  yutil
)

target_sources(cpp-string_utils-quote PRIVATE
<<<<<<< HEAD:src/library/string_utils/quote/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/string_utils/quote/quote.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/string_utils/quote/quote.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/string_utils/quote/CMakeLists.txt
)


