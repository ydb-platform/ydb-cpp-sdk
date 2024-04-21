add_library(library-cpp-colorizer)

target_link_libraries(library-cpp-colorizer PUBLIC
  yutil
)

target_sources(library-cpp-colorizer PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/colorizer/colors.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/colorizer/output.cpp
)
