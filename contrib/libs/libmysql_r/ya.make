# Generated by devtools/yamaker.

LIBRARY()

LICENSE(
    Apache-2.0 AND
    Artistic-1.0-Perl AND
    BSD-2-Clause AND
    BSD-3-Clause AND
    BSD-4-Clause AND
    BSL-1.0 AND
    CC-BY-4.0 AND
    Cmu-Computing-Services AND
    GCC-exception-3.1 AND
    GPL-1.0-only AND
    GPL-1.0-or-later AND
    GPL-2.0-only AND
    GPL-2.0-only WITH Mysql-Linking-Exception-2018 AND
    GPL-2.0-only WITH Universal-FOSS-exception-1.0 AND
    GPL-3.0-only AND
    ICU AND
    ISC AND
    JSON AND
    LGPL AND
    LGPL-2.0-only AND
    LGPL-2.0-or-later AND
    LGPL-2.1-only AND
    LGPL-2.1-or-later AND
    LicenseRef-scancode-other-permissive AND
    MIT AND
    Mit-Old-Style AND
    NAIST-2003 AND
    OLDAP-2.8 AND
    OpenSSL AND
    PSF-3.7.2 AND
    Protobuf-License AND
    Public-Domain AND
    Python-2.0 AND
    Spencer-94 AND
    Stlport-4.5 AND
    Unicode AND
    Unicode-Mappings AND
    Unicode-TOU AND
    Universal-FOSS-exception-1.0 AND
    X11-Lucent AND
    Zlib AND
    bzip2-1.0.6
)

LICENSE_TEXTS(.yandex_meta/licenses.list.txt)

VERSION(8.0.17)

PEERDIR(
    contrib/libs/libmysql_r/dbug
    contrib/libs/libmysql_r/mysys
    contrib/libs/libmysql_r/mysys/mytime
    contrib/libs/libmysql_r/strings
    contrib/libs/libmysql_r/vio
    contrib/libs/openssl
    contrib/libs/zlib
)

ADDINCL(
    GLOBAL contrib/libs/libmysql_r/include
    contrib/libs/libmysql_r
    contrib/libs/libmysql_r/libbinlogevents/include
)

NO_COMPILER_WARNINGS()

NO_UTIL()

CFLAGS(
    -DCLIENT_PROTOCOL_TRACING
    -DDBUG_OFF
    -DHAVE_CONFIG_H
    -DHAVE_LIBEVENT2
    -DHAVE_OPENSSL
    -DHAVE_TLSv13
    -DLZ4_DISABLE_DEPRECATE_WARNINGS
    -DRAPIDJSON_NO_SIZETYPEDEFINE
    -DRAPIDJSON_SCHEMA_USE_INTERNALREGEX=0
    -DRAPIDJSON_SCHEMA_USE_STDREGEX=1
    -DUNISTR_FROM_CHAR_EXPLICIT=explicit
    -DUNISTR_FROM_STRING_EXPLICIT=explicit
)

SRCS(
    libmysql/errmsg.cc
    libmysql/libmysql.cc
    libmysql/mysql_trace.cc
    sql-common/client.cc
    sql-common/client_authentication.cc
    sql-common/client_plugin.cc
    sql-common/get_password.cc
    sql-common/net_serv.cc
    sql/auth/password.cc
    sql/auth/sha2_password_common.cc
)

END()

RECURSE(
    dbug
    mysys
    mysys/mytime
    strings
    strings/uca9dump
    vio
)
