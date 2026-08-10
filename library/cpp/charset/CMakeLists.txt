if (YDB_SDK_TESTS)
  add_ydb_test(NAME charset-lite-ut
    SOURCES
      ci_string_ut.cpp
      codepage_ut.cpp
    LINK_LIBRARIES
      charset-lite
      cpp-testing-unittest_main
    LABELS
      unit
  )
  target_compile_options(charset-lite-ut PRIVATE
    -DLIBRARY_CHARSET_WITHOUT_LIBICONV=yes
  )

  add_ydb_test(NAME charset-ut
    SOURCES
      iconv_ut.cpp
      recyr_int_ut.cpp
      wide_ut.cpp
    LINK_LIBRARIES
      ydb-sdk-charset
      cpp-testing-unittest_main
    LABELS
      unit
  )
endif()

_ydb_sdk_add_library(charset-lite)
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


_ydb_sdk_add_library(ydb-sdk-charset)
set_target_properties(ydb-sdk-charset PROPERTIES EXPORT_NAME charset)
target_link_libraries(ydb-sdk-charset
  PUBLIC
    yutil
    charset-lite
  PRIVATE
    Iconv::Iconv
)
target_sources(ydb-sdk-charset
  PRIVATE
    decodeunknownplane.cpp
    wide.cpp
    iconv.cpp
)
_ydb_sdk_install_targets(TARGETS ydb-sdk-charset)
