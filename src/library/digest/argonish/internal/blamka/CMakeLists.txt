add_library(argonish-internal-blamka INTERFACE)

target_link_libraries(argonish-internal-blamka INTERFACE
  yutil
  argonish-internal-rotations
)
