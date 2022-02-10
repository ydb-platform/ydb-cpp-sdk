OWNER(g:contrib)

RECURSE(
    bdb
    bison
    chromaprinter
    file2c
    flame-graph
    flatc
    flatc64
    flex
    flex-old
    flex-old/fl
    fluent-bit
    fusermount
    gperf
    gprof2dot
    grpc_cli
    kyotocabinet
    langid
    leveldb
    lld
    lldb
    mp4viewer
    open-vcdiff
    open-vcdiff/bin
    plantuml
    protoc
    protoc-c
    protoc_std
    python
    python3
    python3/pycc
    python3/src/Lib/lib2to3
    ragel5
    ragel6
    sancov
    sqlite3
    swig
    tf
    tpce-benchmark
    tpch-benchmark
    tre 
    unbound
    unbound/build
    vowpal_wabbit
    wapiti
    word2vec
    xdelta3
    xsltproc
    yasm
    ycmd
    zookeeper
    jdk
    jdk/test
    xmllint
)

IF (NOT OS_WINDOWS)
    RECURSE(
        ag
        lftp
        make
    )
ENDIF ()
