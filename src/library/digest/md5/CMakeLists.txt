add_library(cpp-digest-md5)

target_link_libraries(cpp-digest-md5 PUBLIC
  yutil
  contrib-libs-nayuki_md5
  cpp-string_utils-base64
)

target_sources(cpp-digest-md5 PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/digest/md5/md5.cpp
)
