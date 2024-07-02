add_library(argonish-internal-argon2 INTERFACE)

target_link_libraries(argonish-internal-argon2 INTERFACE
  yutil
  argonish-internal-blamka
  argonish-internal-blake2b
  threading-poor_man_openmp
)


_ydb_sdk_install_targets(TARGETS argonish-internal-argon2)