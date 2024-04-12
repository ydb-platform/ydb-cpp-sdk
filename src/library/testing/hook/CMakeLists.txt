add_library(cpp-testing-hook)
target_link_libraries(cpp-testing-hook PUBLIC
  yutil
)
target_sources(cpp-testing-hook PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/testing/hook/hook.cpp
)
