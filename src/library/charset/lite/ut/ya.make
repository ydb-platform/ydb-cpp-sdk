UNITTEST_FOR(src/library/charset/lite)

SRCDIR(src/library/charset)

CFLAGS(-DLIBRARY_CHARSET_WITHOUT_LIBICONV=yes)
SRCS(
    ci_string_ut.cpp
    codepage_ut.cpp
)

END()
