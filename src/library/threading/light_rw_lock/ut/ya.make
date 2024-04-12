UNITTEST_FOR(src/library/threading/light_rw_lock)

SRCS(
    rwlock_ut.cpp
)

PEERDIR(
    src/library/deprecated/atomic
)

END()
