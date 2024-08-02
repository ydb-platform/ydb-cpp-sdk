_ydb_sdk_add_library(util-draft)

target_sources(util-draft PRIVATE
  ${YDB_SDK_SOURCE_DIR}/util/draft/date.cpp
  ${YDB_SDK_SOURCE_DIR}/util/draft/datetime.cpp
  ${YDB_SDK_SOURCE_DIR}/util/draft/enum.cpp
  ${YDB_SDK_SOURCE_DIR}/util/draft/holder_vector.cpp
  ${YDB_SDK_SOURCE_DIR}/util/draft/ip.cpp
  ${YDB_SDK_SOURCE_DIR}/util/draft/matrix.cpp
  ${YDB_SDK_SOURCE_DIR}/util/draft/memory.cpp
)
