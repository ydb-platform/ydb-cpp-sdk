add_library(cpp-threading-poor_man_openmp)

target_link_libraries(cpp-threading-poor_man_openmp PUBLIC
  yutil
  cpp-deprecated-atomic
)

target_sources(cpp-threading-poor_man_openmp PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/threading/poor_man_openmp/thread_helper.cpp
)
