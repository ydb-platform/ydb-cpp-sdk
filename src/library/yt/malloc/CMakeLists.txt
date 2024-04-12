add_library(cpp-yt-malloc)

target_link_libraries(cpp-yt-malloc PUBLIC
  yutil
)

target_sources(cpp-yt-malloc PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD:src/library/yt/malloc/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/malloc/malloc.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/yt/malloc/malloc.cpp
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/yt/malloc/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/yt/malloc/malloc.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
)

if (NOT (WIN32 AND CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64"))
  target_compile_options(cpp-yt-malloc PRIVATE
    -Wdeprecated-this-capture
  )
endif ()
