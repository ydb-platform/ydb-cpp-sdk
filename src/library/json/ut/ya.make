UNITTEST_FOR(src/library/json)

PEERDIR(
    src/library/string_utils/relaxed_escaper
)

SRCS(
    json_reader_fast_ut.cpp
    json_reader_ut.cpp
    json_prettifier_ut.cpp
    json_writer_ut.cpp
    json_saveload_ut.cpp
)

END()
