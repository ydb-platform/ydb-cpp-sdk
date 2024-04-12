add_library(cpp-string_utils-relaxed_escaper)

target_link_libraries(cpp-string_utils-relaxed_escaper PUBLIC
  
  yutil
)

target_sources(cpp-string_utils-relaxed_escaper PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/string_utils/relaxed_escaper/relaxed_escaper.cpp
)


