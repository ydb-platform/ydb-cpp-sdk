add_library(cpp-unicode_normalization)

target_link_libraries(cpp-unicode_normalization PUBLIC
  yutil
)

target_sources(cpp-unicode_normalization PRIVATE
  generated/composition.cpp
  generated/decomposition.cpp
  normalization.cpp
)
