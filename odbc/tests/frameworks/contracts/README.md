# Binding boundary contract

The same named scenarios run after each upstream suite, through the unmodified
binding installed from its locked source. They exercise connection, prepared
binding, result metadata, affected rows, transactions, independent connections and
cleanup. Ruby runs both ANSI and UTF-8 extensions. Each case reports independently;
missing records, duplicate records and process failures remain failures.

| Public binding | Common scenarios | Result-set transitions | Concurrent independent connections |
| --- | --- | --- | --- |
| PHP ext/odbc | Yes | `odbc_next_result` | PHP runtime has no thread API |
| PHP PDO_ODBC | Yes | `nextRowset` | PHP runtime has no thread API |
| Qt QODBC | Yes | `nextResult` | Yes |
| Ruby ODBC, both extensions | Yes | `more_results` | Yes |
| R DBI/odbc | Yes | No public advancement API | R API requires its owning thread |
| LuaSQL | Yes | No public advancement API | Lua states require external thread ownership |

The result transition case uses different column names, types and shapes, discards
unread rows, reaches the end and reuses the statement. These are SELECT result sets:
YDB does not expose intermediate DML results or per-statement affected counts.

[R odbc's exports](https://github.com/r-dbi/odbc/blob/v1.4.2/NAMESPACE) and
[LuaSQL's public methods](https://github.com/lunarmodules/luasql/blob/491fea76b9d9ee613fcd7de6b8a1c902e3668d11/src/ls_odbc.c)
provide no result-set advancement operation. No passing or skipped transition case
is generated for those APIs. Their upstream result handling cases still run.

The native `Concurrency` cases cover shared-handle serialization, concurrent
autocommit statements, explicit-transaction serialization, independent connection
progress and cross-thread handle handoff. Those driver properties cannot all be
exercised through thread-affine language APIs. Qt connections remain on the thread
which created them; Ruby uses the binding's calls that release the GVL.

Fixtures and connection modes follow the ordinary consumer job. The scripts add
no source patches, driver wrappers or consumer-specific workarounds to the package.
