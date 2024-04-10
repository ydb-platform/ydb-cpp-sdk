add_library(cpp-getopt-small)

target_link_libraries(cpp-getopt-small PUBLIC
  yutil
  library-cpp-colorizer
  cpp-string_utils-misc
  )

target_sources(cpp-getopt-small PRIVATE
  ${YDB_SDK_SOURCE_DIR}/library/cpp/getopt/small/completer.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/getopt/small/completer_command.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/getopt/small/completion_generator.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/getopt/small/formatted_output.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/getopt/small/last_getopt.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/getopt/small/last_getopt_easy_setup.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/getopt/small/last_getopt_opt.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/getopt/small/last_getopt_opts.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/getopt/small/last_getopt_parser.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/getopt/small/last_getopt_parse_result.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/getopt/small/modchooser.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/getopt/small/opt.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/getopt/small/opt2.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/getopt/small/posix_getopt.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/getopt/small/wrap.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/getopt/small/ygetopt.cpp
)



