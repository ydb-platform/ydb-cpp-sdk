get_built_tool_path(
  TOOL_rescompiler_bin
  TOOL_rescompiler_dependency
  tools/rescompiler/bin
  rescompiler
)

add_library(service-pages-resources INTERFACE)

target_link_libraries(service-pages-resources INTERFACE
  contrib-libs-cxxsupp
  yutil
  library-cpp-resource
  cpp-monlib-dynamic_counters
)

add_global_library_for(service-pages-resources.global service-pages-resources)

target_link_libraries(service-pages-resources.global PUBLIC
  contrib-libs-cxxsupp
  yutil
  library-cpp-resource
  cpp-monlib-dynamic_counters
)
target_sources(service-pages-resources.global PRIVATE
  ${CMAKE_BINARY_DIR}/library/cpp/monlib/service/pages/resources/ff1cb2429e1fce561e9b0ed0d5522c37.cpp
)
resources(service-pages-resources.global
  ${CMAKE_BINARY_DIR}/library/cpp/monlib/service/pages/resources/ff1cb2429e1fce561e9b0ed0d5522c37.cpp
  INPUTS
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/service/pages/resources/static/css/bootstrap.min.css
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/service/pages/resources/static/fonts/glyphicons-halflings-regular.eot
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/service/pages/resources/static/fonts/glyphicons-halflings-regular.svg
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/service/pages/resources/static/fonts/glyphicons-halflings-regular.ttf
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/service/pages/resources/static/fonts/glyphicons-halflings-regular.woff
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/service/pages/resources/static/js/bootstrap.min.js
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/service/pages/resources/static/js/jquery.min.js
  KEYS
  static/css/bootstrap.min.css
  static/fonts/glyphicons-halflings-regular.eot
  static/fonts/glyphicons-halflings-regular.svg
  static/fonts/glyphicons-halflings-regular.ttf
  static/fonts/glyphicons-halflings-regular.woff
  static/js/bootstrap.min.js
  static/js/jquery.min.js
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(service-pages-resources INTERFACE
    contrib-libs-linux-headers
  )
  target_link_libraries(service-pages-resources.global PUBLIC
    contrib-libs-linux-headers
  )
endif()
