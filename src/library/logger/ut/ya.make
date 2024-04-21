UNITTEST()

PEERDIR(
    ADDINCL src/library/logger
    src/library/logger/init_context
    src/library/yconf/patcher
)

SRCDIR(src/library/logger)

SRCS(
    log_ut.cpp
    element_ut.cpp
    rotating_file_ut.cpp
    composite_ut.cpp
    reopen_ut.cpp
)

END()
