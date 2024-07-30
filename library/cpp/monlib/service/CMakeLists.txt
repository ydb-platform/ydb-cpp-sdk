add_library(monlib-service-pages)
target_link_libraries(monlib-service-pages
  PUBLIC
    yutil
    resource
    monlib-metrics
    monlib-service
  PRIVATE
    build_info
    svnversion
    monlib-encode-json
    monlib-encode-text
    monlib-encode-spack
    monlib-encode-prometheus
)
target_sources(monlib-service-pages PRIVATE
  pages/diag_mon_page.cpp
  pages/html_mon_page.cpp
  pages/index_mon_page.cpp
  pages/mon_page.cpp
  pages/pre_mon_page.cpp
  pages/resource_mon_page.cpp
  pages/templates.cpp
  pages/version_mon_page.cpp
  pages/registry_mon_page.cpp
)
_ydb_sdk_install_targets(TARGETS monlib-service-pages)


add_library(monlib-service)
target_link_libraries(monlib-service 
  PUBLIC
    yutil
    monlib-encode
    coroutine-engine
    coroutine-listener
    http-fetch
    http-server
    logger
    cgiparam
  PRIVATE
    http-io
    svnversion
    library-uri
    string_utils-base64
    protobuf::libprotobuf
)
target_sources(monlib-service
  PRIVATE
    monservice.cpp
    mon_service_http_request.cpp
    service.cpp
    format.cpp
    auth.cpp
)
_ydb_sdk_install_targets(TARGETS monlib-service)
