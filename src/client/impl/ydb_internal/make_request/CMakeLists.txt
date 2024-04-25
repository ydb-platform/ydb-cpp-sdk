add_library(impl-ydb_internal-make_request)

target_link_libraries(impl-ydb_internal-make_request PUBLIC
  yutil
  protobuf::libprotobuf
  api-protos
)

target_sources(impl-ydb_internal-make_request PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/client/impl/ydb_internal/make_request/CMakeLists.txt
  make.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/client/impl/ydb_internal/make_request/make.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):client/impl/ydb_internal/make_request/CMakeLists.txt
=======
  make.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):client/impl/ydb_internal/make_request/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  make.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  make.cpp
>>>>>>> origin/main
)
