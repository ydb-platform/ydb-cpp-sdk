add_library(cpp-string_utils-quote)

target_link_libraries(cpp-string_utils-quote PUBLIC
  
  yutil
)

target_sources(cpp-string_utils-quote PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/string_utils/quote/quote.cpp
)


