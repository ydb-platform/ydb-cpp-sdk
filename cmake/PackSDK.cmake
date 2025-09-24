set(CPACK_PACKAGE_VENDOR "ydb.tech")

set(CPACK_PACKAGE_DESCRIPTION
    "YDB C++ SDK\
    YDB is an open source Distributed SQL Database that \
    combines high availability and scalability \
    with strict consistency and ACID transactions."
)

set(CPACK_PACKAGE_NAME "ydb-cpp-sdk-dev")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_VERSION "${YDB_SDK_VERSION}")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://ydb.tech")
set(CPACK_PACKAGE_CONTACT "YDB Team <whoami@where>") # TODO
set(CPACK_RESOURCE_FILE_LICENSE "${YDB_SDK_SOURCE_DIR}/LICENSE") 
# TODO add more parameters

# TODO specify package dependencies
# set(CPACK_DEBIAN_PACKAGE_DEPENDS "liblz4-dev libdouble-conversion-dev libssl-dev")

# TODO use lintian in CI to check deb package
include(CPack)
