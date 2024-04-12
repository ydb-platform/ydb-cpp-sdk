add_subdirectory(common)
add_subdirectory(fast_sax)
add_subdirectory(writer)

add_library(library-cpp-json)

target_link_libraries(library-cpp-json PUBLIC
  yutil
  RapidJSON::RapidJSON
  cpp-json-common
  cpp-json-fast_sax
  cpp-json-writer
  cpp-string_utils-relaxed_escaper
)

target_sources(library-cpp-json PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD:src/library/json/CMakeLists.txt
=======
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
<<<<<<< HEAD:src/library/json/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
<<<<<<< HEAD:src/library/json/CMakeLists.txt
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
  ${YDB_SDK_SOURCE_DIR}/src/library/json/json_writer.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/json/json_reader.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/json/json_prettifier.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/json/rapidjson_helpers.cpp
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/json/json_writer.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/json/json_reader.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/json/json_prettifier.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/json/rapidjson_helpers.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/json/CMakeLists.txt
=======
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/json/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)



