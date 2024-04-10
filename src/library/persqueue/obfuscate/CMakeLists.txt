add_library(library-persqueue-obfuscate)

target_link_libraries(library-persqueue-obfuscate PUBLIC
  yutil
)

target_sources(library-persqueue-obfuscate PRIVATE
<<<<<<< HEAD:src/library/persqueue/obfuscate/CMakeLists.txt
  obfuscate.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/ydb/library/persqueue/obfuscate/obfuscate.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):ydb/library/persqueue/obfuscate/CMakeLists.txt
)
