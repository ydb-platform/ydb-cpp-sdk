add_library(monlib-service-pages)

target_link_libraries(monlib-service-pages PUBLIC
  yutil
  library-build_info
  library-svnversion
  library-resource
  monlib-service
  monlib-encode-json
  monlib-encode-text
  monlib-encode-spack
  monlib-encode-prometheus
)

target_sources(monlib-service-pages PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/service/pages/diag_mon_page.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/service/pages/html_mon_page.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/service/pages/index_mon_page.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/service/pages/mon_page.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/service/pages/pre_mon_page.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/service/pages/resource_mon_page.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/service/pages/templates.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/service/pages/version_mon_page.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/service/pages/registry_mon_page.cpp
)

_ydb_sdk_install_targets(TARGETS monlib-service-pages)
