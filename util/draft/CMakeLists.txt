add_library(util-draft)

target_sources(util-draft PRIVATE
  ${CMAKE_SOURCE_DIR}/util/draft/date.cpp
  ${CMAKE_SOURCE_DIR}/util/draft/datetime.cpp
  ${CMAKE_SOURCE_DIR}/util/draft/enum.cpp
  ${CMAKE_SOURCE_DIR}/util/draft/holder_vector.cpp
  ${CMAKE_SOURCE_DIR}/util/draft/ip.cpp
  ${CMAKE_SOURCE_DIR}/util/draft/matrix.cpp
  ${CMAKE_SOURCE_DIR}/util/draft/memory.cpp
)
