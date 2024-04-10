add_library(cpp-mime-types)

target_link_libraries(cpp-mime-types PUBLIC
  yutil
  tools-enum_parser-enum_serialization_runtime
)

target_sources(cpp-mime-types PRIVATE
<<<<<<< HEAD:src/library/mime/types/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/mime/types/mime.cpp
)

generate_enum_serilization(cpp-mime-types
  ${YDB_SDK_SOURCE_DIR}/include/ydb-cpp-sdk/library/mime/types/mime.h
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/mime/types/mime.cpp
)

generate_enum_serilization(cpp-mime-types
  ${YDB_SDK_SOURCE_DIR}/library/cpp/mime/types/mime.h
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/mime/types/CMakeLists.txt
  INCLUDE_HEADERS
  include/ydb-cpp-sdk/library/mime/types/mime.h
)
