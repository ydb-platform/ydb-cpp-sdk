_ydb_sdk_add_library(impl-internal-local_dc_detector)

target_link_libraries(impl-internal-local_dc_detector PUBLIC
  yutil
  api-protos
  neh-asio
  threading-future
)

target_sources(impl-internal-local_dc_detector PRIVATE
  local_dc_detector.cpp
  pinger.cpp
)

_ydb_sdk_install_targets(TARGETS impl-internal-local_dc_detector)
