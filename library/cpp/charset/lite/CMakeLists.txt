add_library(cpp-charset-lite)

target_link_libraries(cpp-charset-lite PUBLIC
  cpp-string_utils-misc
  yutil
)

target_sources(cpp-charset-lite PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/charset/generated/cp_data.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/charset/generated/encrec_data.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/charset/codepage.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/charset/cp_encrec.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/charset/doccodes.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/charset/ci_string.cpp
)
