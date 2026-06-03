set(CPACK_PACKAGE_VENDOR "ydb.tech")

set(CPACK_PACKAGE_DESCRIPTION
    "YDB C++ SDK\
    YDB is an open source Distributed SQL Database that \
    combines high availability and scalability \
    with strict consistency and ACID transactions."
)

set(CPACK_PACKAGE_NAME "ydb-cpp-sdk")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_VERSION "${YDB_SDK_VERSION}")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://ydb.tech")
set(CPACK_PACKAGE_CONTACT "YDB Team <info@ydb.tech>")
set(CPACK_RESOURCE_FILE_LICENSE "${YDB_SDK_SOURCE_DIR}/LICENSE") 

set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL libydb-cpp libydb-cpp-iam libydb-cpp-otel-metrics libydb-cpp-otel-tracing)

set(CPACK_DEBIAN_LIBYDB_CPP_PACKAGE_NAME "libydb-cpp-dev")
set(CPACK_DEBIAN_LIBYDB_CPP_PACKAGE_DEPENDS
  "libidn11-dev, libssl-dev, zlib1g-dev, libprotobuf-dev, libgrpc++-dev, libbrotli-dev, liblz4-dev, libzstd-dev, libbz2-dev, libxxhash-dev, libsnappy-dev, libdouble-conversion-dev, libgtest-dev, libre2-dev, libc-ares-dev, rapidjson-dev"
)

set(CPACK_DEBIAN_LIBYDB_CPP_IAM_PACKAGE_NAME "libydb-cpp-iam-dev")
set(CPACK_DEBIAN_LIBYDB_CPP_IAM_PACKAGE_DEPENDS "libydb-cpp-dev (= ${YDB_SDK_VERSION})")

set(CPACK_DEBIAN_LIBYDB_CPP_OTEL_METRICS_PACKAGE_NAME "libydb-cpp-otel-metrics-dev")
set(CPACK_DEBIAN_LIBYDB_CPP_OTEL_METRICS_PACKAGE_DEPENDS "libydb-cpp-dev (= ${YDB_SDK_VERSION})")

set(CPACK_DEBIAN_LIBYDB_CPP_OTEL_TRACING_PACKAGE_NAME "libydb-cpp-otel-tracing-dev")
set(CPACK_DEBIAN_LIBYDB_CPP_OTEL_TRACING_PACKAGE_DEPENDS "libydb-cpp-dev (= ${YDB_SDK_VERSION})")

foreach(component IN ITEMS libydb-cpp libydb-cpp-iam libydb-cpp-otel-metrics libydb-cpp-otel-tracing)
  string(TOUPPER "${component}" component_upper)
  string(REPLACE "-" "_" component_var "${component_upper}")
  set("CPACK_DEBIAN_${component_upper}_PACKAGE_NAME" "${CPACK_DEBIAN_${component_var}_PACKAGE_NAME}")
  set("CPACK_DEBIAN_${component_upper}_PACKAGE_DEPENDS" "${CPACK_DEBIAN_${component_var}_PACKAGE_DEPENDS}")
endforeach()

include(CPack)
