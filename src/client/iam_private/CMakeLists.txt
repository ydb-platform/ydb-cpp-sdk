add_library(cpp-client-iam_private)

target_link_libraries(cpp-client-iam_private PUBLIC
  yutil
  client-yc_private-iam
  client-iam-common
)

target_sources(cpp-client-iam_private PRIVATE
  iam.cpp
)

add_library(YDB-CPP-SDK::IamPrivate ALIAS cpp-client-iam_private)
