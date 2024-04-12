FUZZ()

FUZZ_OPTS(-rss_limit_mb=1024)

SIZE(MEDIUM)

PEERDIR(
    src/library/monlib/encode/spack
    src/library/monlib/encode/fake
)

SRCS(
    main.cpp
)

END()
