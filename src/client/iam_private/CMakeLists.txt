add_library(client-iam_private)

target_link_libraries(client-iam_private PUBLIC
  yutil
  api-client-yc_private
  client-iam-common
)

target_sources(client-iam_private PRIVATE
  iam.cpp
)

_ydb_sdk_install_targets(TARGETS client-iam_private)

add_library(YDB-CPP-SDK::IamPrivate ALIAS client-iam_private)
