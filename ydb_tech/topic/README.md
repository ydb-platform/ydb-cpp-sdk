# Topic examples for ydb.tech

This executable supplies the C++ snippets in the topic reference on ydb.tech.
It links to the SDK built from this checkout.

With the SDK build prerequisites and a local YDB instance, run from the repository root:

```sh
cmake --preset release-clang -D YDB_SDK_EXAMPLES=ON
cmake --build --preset default --target ydb_tech_topic
timeout 180s build/ydb_tech/topic/ydb_tech_topic
```

`YDB_ENDPOINT` defaults to `localhost:2136` and `YDB_DATABASE` to `/local`.
The application creates unique topics, verifies all received payloads, tests writer
and reader variants and transactions, and removes its topics afterwards.
