add_library(cpp-json-fast_sax)

target_link_libraries(cpp-json-fast_sax PUBLIC
  yutil
  cpp-json-common
)

target_sources(cpp-json-fast_sax PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/library/json/fast_sax/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/json/fast_sax/unescape.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/json/fast_sax/unescape.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/json/fast_sax/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/json/fast_sax/unescape.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/json/fast_sax/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)

if (WIN32 AND CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
  target_ragel_lexers(cpp-json-fast_sax
    PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/library/json/fast_sax/CMakeLists.txt
    ${YDB_SDK_SOURCE_DIR}/src/library/json/fast_sax/parser.rl6
=======
    ${YDB_SDK_SOURCE_DIR}/library/cpp/json/fast_sax/parser.rl6
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/json/fast_sax/CMakeLists.txt
=======
    ${YDB_SDK_SOURCE_DIR}/src/library/json/fast_sax/parser.rl6
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/json/fast_sax/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
    -CT0
  )
else ()
  target_ragel_lexers(cpp-json-fast_sax
    PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/library/json/fast_sax/CMakeLists.txt
    ${YDB_SDK_SOURCE_DIR}/src/library/json/fast_sax/parser.rl6
=======
    ${YDB_SDK_SOURCE_DIR}/library/cpp/json/fast_sax/parser.rl6
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/json/fast_sax/CMakeLists.txt
=======
    ${YDB_SDK_SOURCE_DIR}/src/library/json/fast_sax/parser.rl6
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/json/fast_sax/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
    -CG2
  )
endif ()
