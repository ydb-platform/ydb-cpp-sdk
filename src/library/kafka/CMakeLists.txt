_ydb_sdk_add_library(library-kafka)

target_link_libraries(library-kafka PUBLIC
  yutil
  streams-zstd
)

target_sources(library-kafka PRIVATE
  kafka_messages_int.cpp
  kafka_records.cpp
  kafka_write_buffer.cpp
)

_ydb_sdk_install_targets(TARGETS library-kafka)
