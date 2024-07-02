add_library(library-charset-lite)

target_link_libraries(library-charset-lite PUBLIC
  string_utils-misc
  yutil
)

target_sources(library-charset-lite PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/charset/generated/cp_data.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/charset/generated/encrec_data.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/charset/codepage.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/charset/cp_encrec.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/charset/doccodes.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/charset/ci_string.cpp
)

_ydb_sdk_install_targets(TARGETS library-charset-lite)
