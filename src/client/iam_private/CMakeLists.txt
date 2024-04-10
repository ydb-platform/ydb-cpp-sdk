add_library(cpp-client-iam_private)

target_link_libraries(cpp-client-iam_private PUBLIC
  yutil
  client-yc_private-iam
  client-iam-common
)

target_sources(cpp-client-iam_private PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/client/iam_private/CMakeLists.txt
  iam.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/iam_private/iam.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/iam_private/CMakeLists.txt
=======
  iam.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/iam_private/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)

add_library(YDB-CPP-SDK::IamPrivate ALIAS cpp-client-iam_private)
