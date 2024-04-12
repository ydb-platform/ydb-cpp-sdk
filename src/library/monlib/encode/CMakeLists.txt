add_subdirectory(buffered)
add_subdirectory(json)
add_subdirectory(legacy_protobuf)
add_subdirectory(prometheus)
add_subdirectory(spack)
add_subdirectory(text)

add_library(cpp-monlib-encode)
target_link_libraries(cpp-monlib-encode PUBLIC
  yutil
  cpp-monlib-metrics
  tools-enum_parser-enum_serialization_runtime
  cpp-string_utils-misc
)
target_sources(cpp-monlib-encode PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/monlib/encode/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/encoder.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/encoder_state.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/format.cpp
)
generate_enum_serilization(cpp-monlib-encode
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/encoder_state_enum.h
  GEN_HEADER
  ${YDB_SDK_BINARY_DIR}/src/library/monlib/encode/encoder_state_enum.h_serialized.h
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/encode/encoder.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/encode/encoder_state.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/encode/format.cpp
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/encoder.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/encoder_state.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/format.cpp
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)
generate_enum_serilization(cpp-monlib-encode
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/encoder_state_enum.h
  GEN_HEADER
<<<<<<< HEAD
  ${YDB_SDK_BINARY_DIR}/library/cpp/monlib/encode/encoder_state_enum.h_serialized.h
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/monlib/encode/CMakeLists.txt
=======
  ${YDB_SDK_BINARY_DIR}/src/library/monlib/encode/encoder_state_enum.h_serialized.h
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)
generate_enum_serilization(cpp-monlib-encode
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/encoder_state_enum.h
  GEN_HEADER
<<<<<<< HEAD
  ${YDB_SDK_BINARY_DIR}/library/cpp/monlib/encode/encoder_state_enum.h_serialized.h
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/monlib/encode/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  ${YDB_SDK_BINARY_DIR}/src/library/monlib/encode/encoder_state_enum.h_serialized.h
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
  INCLUDE_HEADERS
  src/library/monlib/encode/encoder_state_enum.h
)


