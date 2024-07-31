add_library(yson)

target_link_libraries(yson 
  PUBLIC
    yutil
    yt-misc
    yt-yson
)

target_sources(yson 
  PRIVATE
    consumer.cpp
    lexer.cpp
    parser.cpp
    token.cpp
    tokenizer.cpp
    varint.cpp
    writer.cpp
)

_ydb_sdk_install_targets(TARGETS yson)
