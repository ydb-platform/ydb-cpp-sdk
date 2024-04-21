add_library(cpp-testing-hook)

target_link_libraries(cpp-testing-hook PUBLIC
  yutil
)

target_sources(cpp-testing-hook PRIVATE
  hook.cpp
)
