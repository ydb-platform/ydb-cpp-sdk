add_library(cpp-client-resources)
target_link_libraries(cpp-client-resources PUBLIC
  yutil
  library-cpp-resource
)
target_sources(cpp-client-resources PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/client/resources/CMakeLists.txt
  ydb_resources.cpp
  ydb_ca.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/resources/ydb_resources.cpp
  ${YDB_SDK_SOURCE_DIR}/client/resources/ydb_ca.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/resources/CMakeLists.txt
=======
  ydb_resources.cpp
  ydb_ca.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/resources/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ydb_resources.cpp
  ydb_ca.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ydb_resources.cpp
  ydb_ca.cpp
>>>>>>> origin/main
)

add_global_library_for(cpp-client-resources.global cpp-client-resources)
target_link_libraries(cpp-client-resources.global PUBLIC
  yutil
  library-cpp-resource
)
target_sources(cpp-client-resources.global PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/client/resources/CMakeLists.txt
=======
>>>>>>> origin/main
  ${YDB_SDK_BINARY_DIR}/src/client/resources/6ed212bf45019efe2a5e72b6d5ed50fb.cpp
)
resources(cpp-client-resources.global
  ${YDB_SDK_BINARY_DIR}/src/client/resources/6ed212bf45019efe2a5e72b6d5ed50fb.cpp
  INPUTS
  ${YDB_SDK_SOURCE_DIR}/src/client/resources/ydb_sdk_version.txt
  ${YDB_SDK_SOURCE_DIR}/src/client/resources/ydb_root_ca.pem
<<<<<<< HEAD
=======
  ${YDB_SDK_BINARY_DIR}/client/resources/6ed212bf45019efe2a5e72b6d5ed50fb.cpp
<<<<<<< HEAD
<<<<<<< HEAD
=======
  ${YDB_SDK_BINARY_DIR}/src/client/resources/6ed212bf45019efe2a5e72b6d5ed50fb.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)
resources(cpp-client-resources.global
  ${YDB_SDK_BINARY_DIR}/src/client/resources/6ed212bf45019efe2a5e72b6d5ed50fb.cpp
  INPUTS
<<<<<<< HEAD
  ${YDB_SDK_SOURCE_DIR}/client/resources/ydb_sdk_version.txt
  ${YDB_SDK_SOURCE_DIR}/client/resources/ydb_root_ca.pem
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/resources/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/resources/ydb_sdk_version.txt
  ${YDB_SDK_SOURCE_DIR}/src/client/resources/ydb_root_ca.pem
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
=======
=======
  ${YDB_SDK_BINARY_DIR}/src/client/resources/6ed212bf45019efe2a5e72b6d5ed50fb.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)
resources(cpp-client-resources.global
  ${YDB_SDK_BINARY_DIR}/src/client/resources/6ed212bf45019efe2a5e72b6d5ed50fb.cpp
  INPUTS
<<<<<<< HEAD
  ${YDB_SDK_SOURCE_DIR}/client/resources/ydb_sdk_version.txt
  ${YDB_SDK_SOURCE_DIR}/client/resources/ydb_root_ca.pem
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/resources/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_SOURCE_DIR}/src/client/resources/ydb_sdk_version.txt
  ${YDB_SDK_SOURCE_DIR}/src/client/resources/ydb_root_ca.pem
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main
  KEYS
  ydb_sdk_version.txt
  ydb_root_ca.pem
)

add_library(YDB-CPP-SDK::Resources ALIAS cpp-client-resources)
