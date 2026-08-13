add_ydb_test(NAME library-kafka-messages_ut
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}
  SOURCES
    ${YDB_SDK_SOURCE_DIR}/src/library/kafka/ut/kafka_messages_int_ut.cpp
  LINK_LIBRARIES
    library-kafka
  LABELS
    unit
)

add_ydb_test(NAME library-kafka-records_ut
  INCLUDE_DIRS
    ${YDB_SDK_SOURCE_DIR}
  SOURCES
    ${YDB_SDK_SOURCE_DIR}/src/library/kafka/ut/kafka_records_ut.cpp
  LINK_LIBRARIES
    library-kafka
  LABELS
    unit
)
