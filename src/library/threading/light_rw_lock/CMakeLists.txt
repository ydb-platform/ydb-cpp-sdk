add_library(cpp-threading-light_rw_lock)

target_link_libraries(cpp-threading-light_rw_lock PUBLIC
  yutil
)

target_sources(cpp-threading-light_rw_lock PRIVATE
  ${YDB_SDK_SOURCE_DIR}/src/library/threading/light_rw_lock/lightrwlock.cpp
)
