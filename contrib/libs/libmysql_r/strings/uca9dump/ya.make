# Generated by devtools/yamaker.

IF (USE_PREBUILT_TOOLS)
    INCLUDE(${ARCADIA_ROOT}/build/prebuilt/contrib/libs/libmysql_r/strings/uca9dump/ya.make.prebuilt)
ENDIF()

IF (NOT PREBUILT)
    INCLUDE(${ARCADIA_ROOT}/contrib/libs/libmysql_r/strings/uca9dump/bin/ya.make)
ENDIF()

RECURSE(
    bin
)
