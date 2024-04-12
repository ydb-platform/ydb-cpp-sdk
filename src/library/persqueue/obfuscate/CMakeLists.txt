add_library(library-persqueue-obfuscate)

target_link_libraries(library-persqueue-obfuscate PUBLIC
  yutil
)

target_sources(library-persqueue-obfuscate PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
<<<<<<< HEAD:src/library/persqueue/obfuscate/CMakeLists.txt
  obfuscate.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/ydb/library/persqueue/obfuscate/obfuscate.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/library/persqueue/obfuscate/CMakeLists.txt
=======
  obfuscate.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/library/persqueue/obfuscate/CMakeLists.txt
<<<<<<< HEAD
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
=======
=======
  obfuscate.cpp
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
)
