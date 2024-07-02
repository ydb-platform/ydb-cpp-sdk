add_library(digest-md5)

target_link_libraries(digest-md5 PUBLIC
  yutil
  nayuki_md5
  string_utils-base64
)

target_sources(digest-md5 PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/md5/md5.cpp
)

_ydb_sdk_install_targets(TARGETS digest-md5)
