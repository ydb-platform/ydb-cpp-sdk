OWNER(
    g:contrib
    g:cpp-contrib
)

RECURSE(
    abseil-cpp
    abseil-cpp-tstring
    alsa-lib
    avahi
    aws
    blis
    boost
    cityhash-1.0.2
    cmph
    cpuinfo
    exiv2
    expected-lite
    fast_float
    dragonbox
    gemmlowp
    glib
    glib-networking
    glibmm
    google
    googletest
    gst-plugins-bad
    gst-plugins-base
    gst-plugins-good
    gstreamer
    http-parser
    keyutils
    libelf
    libffi
    libiscsi
    libntirpc
    librseq
    libsigcxx
    libsoup
    libtorrent
    liburcu
    libxsmm
    llhttp
    mpg123
    murmurhash
    nfs_ganesha
    noc
    openal-soft
    patched
    protobuf-c
    protoc-c
    protozero
    range-v3 
    rnnoise
    rubberband
    spdlog
    thrift
    thrift/compiler
    turbo_base64
    uriparser
)

IF(OS_LINUX OR OS_DARWIN)
    RECURSE(
        boost/libs/python
        boost/libs/python/arcadia_test
    )
ENDIF()

IF(OS_ANDROID)
    RECURSE(ashmem)
ENDIF()
