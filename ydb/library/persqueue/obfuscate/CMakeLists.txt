add_library(library-persqueue-obfuscate)

target_link_libraries(library-persqueue-obfuscate PUBLIC
  contrib-libs-cxxsupp
  yutil
)

target_sources(library-persqueue-obfuscate PRIVATE
  ${CMAKE_SOURCE_DIR}/ydb/library/persqueue/obfuscate/obfuscate.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(library-persqueue-obfuscate PUBLIC
    contrib-libs-linux-headers
  )
endif()
