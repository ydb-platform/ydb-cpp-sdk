#pragma once 

#include <src/client/ydb_persqueue_core/ut/ut_utils/test_server.h>

#include <src/client/ydb_topic/impl/write_session.h>

namespace NYdb::NTopic::NTests {

#define TEST_CASE_NAME (this->Name_)

inline static const std::string TEST_TOPIC = "test-topic";
inline static const std::string TEST_CONSUMER = "test-consumer";
inline static const std::string TEST_MESSAGE_GROUP_ID = "test-message_group_id";

class TTopicSdkTestSetup {
public:
    TTopicSdkTestSetup(const std::string& testCaseName, const NKikimr::Tests::TServerSettings& settings = MakeServerSettings(), bool createTopic = true);

    void CreateTopic(const std::string& path = TEST_TOPIC, const std::string& consumer = TEST_CONSUMER, size_t partitionCount = 1);

    std::string GetEndpoint() const;
    std::string GetTopicPath(const std::string& name = TEST_TOPIC) const;
    std::string GetTopicParent() const;
    std::string GetDatabase() const;

    ::NPersQueue::TTestServer& GetServer();
    NActors::TTestActorRuntime& GetRuntime();
    TLog& GetLog();

    TTopicClient MakeClient() const;

    TDriver MakeDriver() const;
    TDriver MakeDriver(const TDriverConfig& config) const;

    TDriverConfig MakeDriverConfig() const;
    static NKikimr::Tests::TServerSettings MakeServerSettings();
private:
    std::string Database;
    ::NPersQueue::TTestServer Server;

    TLog Log = CreateLogBackend("cerr", ELogPriority::TLOG_DEBUG);
};

}