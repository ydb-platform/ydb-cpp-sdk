LIBRARY()

LICENSE(MIT)

LICENSE_TEXTS(.yandex_meta/licenses.list.txt)

OWNER( 
    pg
    g:contrib
    g:cpp-contrib
) 

NO_UTIL()

NO_COMPILER_WARNINGS()

ADDINCL(GLOBAL contrib/libs/brotli/include)

PEERDIR(
    contrib/libs/brotli/common
)

SRCS(
    bit_reader.c
    decode.c
    huffman.c
    state.c
)

END()
