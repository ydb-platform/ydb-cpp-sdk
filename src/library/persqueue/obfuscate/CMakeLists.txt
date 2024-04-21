add_library(library-persqueue-obfuscate)

target_link_libraries(library-persqueue-obfuscate PUBLIC
  yutil
)

target_sources(library-persqueue-obfuscate PRIVATE
  obfuscate.cpp
)
