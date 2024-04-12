UNITTEST_FOR(src/library/digest/argonish)

PEERDIR(
    src/library/digest/argonish
)

SRCS(
    ut.cpp
)

TAG(
    sb:intel_e5_2660v4
    ya:fat
    ya:force_sandbox
)

SIZE(LARGE)

END()
