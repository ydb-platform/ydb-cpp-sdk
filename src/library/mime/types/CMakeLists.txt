add_library(cpp-mime-types)

target_link_libraries(cpp-mime-types PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
)

target_sources(cpp-mime-types PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD:src/library/mime/types/CMakeLists.txt
=======
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
<<<<<<< HEAD:src/library/mime/types/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
<<<<<<< HEAD:src/library/mime/types/CMakeLists.txt
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
>>>>>>> origin/main
  ${YDB_SDK_SOURCE_DIR}/src/library/mime/types/mime.cpp
)

generate_enum_serilization(cpp-mime-types
<<<<<<< HEAD
<<<<<<< HEAD
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/library/mime/types/mime.h
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/mime/types/mime.cpp
)

generate_enum_serilization(cpp-mime-types
  ${YDB_SDK_SOURCE_DIR}/library/cpp/mime/types/mime.h
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/mime/types/CMakeLists.txt
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/mime/types/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/library/mime/types/mime.h
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/mime/types/mime.h
  INCLUDE_HEADERS
  src/library/mime/types/mime.h
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/library/mime/types/mime.h
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/library/mime/types/mime.h
>>>>>>> origin/main
)
