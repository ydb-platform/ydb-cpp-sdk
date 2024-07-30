add_library(charset-lite)
target_link_libraries(charset-lite PUBLIC
  yutil
)
target_sources(charset-lite
  PRIVATE
    generated/cp_data.cpp
    generated/encrec_data.cpp
    codepage.cpp
    cp_encrec.cpp
    doccodes.cpp
    ci_string.cpp
)
_ydb_sdk_install_targets(TARGETS charset-lite)


add_library(charset)
target_link_libraries(charset 
  PUBLIC
    yutil
    charset-lite
  PRIVATE
    Iconv::Iconv
)
target_sources(charset
  PRIVATE
    decodeunknownplane.cpp
    wide.cpp
    iconv.cpp
)
_ydb_sdk_install_targets(TARGETS charset)
