add_subdirectory(common)

add_library(cpp-client-iam)

target_link_libraries(cpp-client-iam PUBLIC
  yutil
  client-yc_public-iam
  client-iam-common
)

target_sources(cpp-client-iam PRIVATE
<<<<<<< HEAD:src/client/iam/CMakeLists.txt
  iam.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/iam/iam.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/iam/CMakeLists.txt
)

add_library(YDB-CPP-SDK::Iam ALIAS cpp-client-iam)
