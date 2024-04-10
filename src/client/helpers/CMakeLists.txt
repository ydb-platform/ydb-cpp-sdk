add_library(cpp-client-helpers)

target_link_libraries(cpp-client-helpers PUBLIC
  yutil
  cpp-client-iam
  client-ydb_types-credentials
  public-issue-protos
)

target_sources(cpp-client-helpers PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/client/helpers/CMakeLists.txt
  helpers.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/helpers/helpers.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/helpers/CMakeLists.txt
=======
  helpers.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/helpers/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)

add_library(YDB-CPP-SDK::Helpers ALIAS cpp-client-helpers)
