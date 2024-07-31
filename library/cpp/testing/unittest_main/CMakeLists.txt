add_library(cpp-testing-unittest_main)

target_link_libraries(cpp-testing-unittest_main PUBLIC
  yutil
  cpp-testing-unittest
  terminate_handler
)

target_sources(cpp-testing-unittest_main PRIVATE
  main.cpp
)
