add_subdirectory(resources)
add_subdirectory(tablesorter)

add_library(monlib-service-pages)

target_link_libraries(monlib-service-pages PUBLIC
  yutil
  library-cpp-build_info
  cpp-malloc-api
  library-cpp-svnversion
  library-cpp-resource
  cpp-monlib-service
  monlib-encode-json
  monlib-encode-text
  monlib-encode-spack
  monlib-encode-prometheus
)

target_sources(monlib-service-pages PRIVATE
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/service/pages/diag_mon_page.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/service/pages/html_mon_page.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/service/pages/index_mon_page.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/service/pages/mon_page.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/service/pages/pre_mon_page.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/service/pages/resource_mon_page.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/service/pages/templates.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/service/pages/version_mon_page.cpp
  ${CMAKE_SOURCE_DIR}/library/cpp/monlib/service/pages/registry_mon_page.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HAVE_CUDA)
  target_link_libraries(monlib-service-pages PUBLIC
    
  )
endif()
