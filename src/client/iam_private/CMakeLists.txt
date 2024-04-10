add_library(cpp-client-iam_private)

target_link_libraries(cpp-client-iam_private PUBLIC
  yutil
  client-yc_private-iam
  client-iam-common
)

target_sources(cpp-client-iam_private PRIVATE
<<<<<<< HEAD:src/client/iam_private/CMakeLists.txt
  iam.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/iam_private/iam.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/iam_private/CMakeLists.txt
)

add_library(YDB-CPP-SDK::IamPrivate ALIAS cpp-client-iam_private)
