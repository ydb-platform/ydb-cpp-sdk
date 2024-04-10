add_library(monlib-encode-prometheus)

target_link_libraries(monlib-encode-prometheus PUBLIC
  yutil
  cpp-monlib-encode
  monlib-encode-buffered
)

target_sources(monlib-encode-prometheus PRIVATE
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
<<<<<<< HEAD:src/library/monlib/encode/prometheus/CMakeLists.txt
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/prometheus/prometheus_decoder.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/prometheus/prometheus_encoder.cpp
=======
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/encode/prometheus/prometheus_decoder.cpp
  ${YDB_SDK_SOURCE_DIR}/library/cpp/monlib/encode/prometheus/prometheus_encoder.cpp
<<<<<<< HEAD
>>>>>>> b509d53abd (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/monlib/encode/prometheus/CMakeLists.txt
=======
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/prometheus/prometheus_decoder.cpp
  ${YDB_SDK_SOURCE_DIR}/src/library/monlib/encode/prometheus/prometheus_encoder.cpp
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 826c9ae11c (Fix CMAKE_SOURCE_DIR bug (#135)):library/cpp/monlib/encode/prometheus/CMakeLists.txt
>>>>>>> 61c08f3f93 (Fix CMAKE_SOURCE_DIR bug (#135))
)


