UNITTEST()

PEERDIR(
    ADDINCL src/library/json/writer
)

SRCDIR(src/library/json/writer)

SRCS(
    json_ut.cpp
    json_value_ut.cpp
)

END()
