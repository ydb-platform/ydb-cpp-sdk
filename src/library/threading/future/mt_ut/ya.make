UNITTEST_FOR(src/library/threading/future)

SRCS(
    future_mt_ut.cpp
)

IF(SANITIZER_TYPE)
    SIZE(MEDIUM)
ENDIF()


END()
