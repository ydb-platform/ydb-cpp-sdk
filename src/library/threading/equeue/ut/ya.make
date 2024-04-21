UNITTEST()

PEERDIR(
    ADDINCL src/library/threading/equeue
    src/library/threading/equeue/fast
)

SRCDIR(src/library/threading/equeue)

SRCS(
    equeue_ut.cpp
)

END()
