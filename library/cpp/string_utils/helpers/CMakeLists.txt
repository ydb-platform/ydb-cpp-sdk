add_library(cpp-string_utils-helpers)

target_sources(cpp-string_utils-helpers PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/string_utils/helpers/helpers.cpp
)
