#pragma once

#include "options.h"

<<<<<<< HEAD
#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/topic/topic.h>
#include <ydb-cpp-sdk/client/table/table.h>
=======
#include <src/client/ydb_driver/driver.h>
#include <src/client/ydb_topic/topic.h>
#include <src/client/ydb_table/table.h>
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))

#include <memory>
#include <optional>

class TApplication {
public:
    explicit TApplication(const TOptions& options);

    void Run();
    void Stop();
    void Finalize();

private:
    struct TRow {
        TRow() = default;
        TRow(ui64 key, const std::string& value);

        ui64 Key = 0;
        std::string Value;
    };

    void CreateTopicReadSession(const TOptions& options);
    void CreateTableSession();

    void BeginTransaction();
    void CommitTransaction();

    void TryCommitTransaction();

    void InsertRowsIntoTable();
    void AppendTableRow(const NYdb::NTopic::TReadSessionEvent::TDataReceivedEvent::TMessage& message);

    std::optional<NYdb::TDriver> Driver;
    std::optional<NYdb::NTopic::TTopicClient> TopicClient;
    std::optional<NYdb::NTable::TTableClient> TableClient;
    std::shared_ptr<NYdb::NTopic::IReadSession> ReadSession;
    std::optional<NYdb::NTable::TSession> TableSession;
    std::optional<NYdb::NTable::TTransaction> Transaction;
    std::vector<NYdb::NTopic::TReadSessionEvent::TStopPartitionSessionEvent> PendingStopEvents;
    std::vector<TRow> Rows;
    std::string TablePath;
};
