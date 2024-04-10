add_library(library-persqueue-obfuscate)

target_link_libraries(library-persqueue-obfuscate PUBLIC
  yutil
)

target_sources(library-persqueue-obfuscate PRIVATE
  ${YDB_SDK_SOURCE_DIR}/ydb/library/persqueue/obfuscate/obfuscate.cpp
)
