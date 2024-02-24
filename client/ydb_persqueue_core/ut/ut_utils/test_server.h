#pragma once
#include <ydb/core/testlib/test_pq_client.h>
#include <client/ydb_persqueue_core/persqueue.h>

#include <ydb/library/grpc/server/grpc_server.h>

#include <library/cpp/testing/unittest/registar.h>
#include <library/cpp/testing/unittest/tests_data.h>

#include <util/system/tempfile.h>

namespace NPersQueue {

static constexpr int DEBUG_LOG_LEVEL = 7;

class TTestServer {
public:
    TTestServer(const NKikimr::Tests::TServerSettings& settings, 
                bool start = true,
                const std::vector<NKikimrServices::EServiceKikimr>& logServices = TTestServer::LOGGED_SERVICES,
                NActors::NLog::EPriority logPriority = NActors::NLog::PRI_DEBUG,
                std::optional<TSimpleSharedPtr<TPortManager>> portManager = std::nullopt)
        : PortManager(portManager.GetOrElse(MakeSimpleShared<TPortManager>()))
        , Port(PortManager->GetPort(2134))
        , GrpcPort(PortManager->GetPort(2135))
        , ServerSettings(settings)
        , GrpcServerOptions(NYdbGrpc::TServerOptions().SetHost("[::1]").SetPort(GrpcPort))
    {
        auto loggerInitializer = [logServices, logPriority](NActors::TTestActorRuntime& runtime) {
            for (auto s : logServices)
                runtime.SetLogPriority(s, logPriority);
        };
        ServerSettings.SetLoggerInitializer(loggerInitializer);

        ServerSettings.Port = Port;
        ServerSettings.SetGrpcPort(GrpcPort);

        if (start)
            StartServer();
    }

    TTestServer(bool start = true)
        : TTestServer(NKikimr::NPersQueueTests::PQSettings(), start)
    {
    }

    void StartServer(bool doClientInit = true, std::optional<TString> databaseName = std::nullopt) {
        Log.SetFormatter([](ELogPriority priority, TStringBuf message) {
            return TStringBuilder() << TInstant::Now() << " " << priority << ": " << message << Endl;
        });

        PrepareNetDataFile();

        CleverServer = MakeHolder<NKikimr::Tests::TServer>(ServerSettings);
        CleverServer->EnableGRpc(GrpcServerOptions);

        Log << TLOG_INFO << "TTestServer started on Port " << Port << " GrpcPort " << GrpcPort;

        AnnoyingClient = MakeHolder<NKikimr::NPersQueueTests::TFlatMsgBusPQClient>(ServerSettings, GrpcPort, databaseName);
        if (doClientInit) {
            AnnoyingClient->FullInit();
            AnnoyingClient->CheckClustersList(CleverServer->GetRuntime());
        }
    }

    void ShutdownGRpc() {
        CleverServer->ShutdownGRpc();
    }

    void EnableGRpc() {
        CleverServer->EnableGRpc(GrpcServerOptions);
    }

    void ShutdownServer() {
        CleverServer = nullptr;
    }

    void RestartServer() {
        ShutdownServer();
        StartServer();
    }

    auto GetRuntime() {
        return CleverServer->GetRuntime();
    }

    void EnableLogs(const std::vector<NKikimrServices::EServiceKikimr>& services = LOGGED_SERVICES,
                    NActors::NLog::EPriority prio = NActors::NLog::PRI_DEBUG) {
        Y_ABORT_UNLESS(CleverServer != nullptr, "Start server before enabling logs");
        for (auto s : services) {
            CleverServer->GetRuntime()->SetLogPriority(s, prio);
        }
    }

    void WaitInit(const TString& topic) {
        AnnoyingClient->WaitTopicInit(topic);
    }

    bool PrepareNetDataFile(const TString& content = "::1/128\tdc1") {
        if (NetDataFile)
            return false;
        NetDataFile = MakeHolder<TTempFileHandle>();
        NetDataFile->Write(content.Data(), content.Size());
        NetDataFile->FlushData();
        ServerSettings.NetClassifierConfig.SetNetDataFilePath(NetDataFile->Name());
        return true;
    }

    void UpdateDC(const TString& name, bool local, bool enabled) {
        AnnoyingClient->UpdateDC(name, local, enabled);
    }

    const NYdb::TDriver& GetDriver() const {
        return CleverServer->GetDriver();
    }

    void KillTopicPqrbTablet(const TString& topicPath) {
        KillTopicTablets(topicPath, true, false);
    }

    void KillTopicPqTablets(const TString& topicPath) {
        KillTopicTablets(topicPath, false, true);
    }

private:
    void KillTopicTablets(const TString& topicPath, bool killPqrb, bool killPq) {
        auto describeResult = AnnoyingClient->Ls(topicPath);
        UNIT_ASSERT_C(describeResult->Record.GetPathDescription().HasPersQueueGroup(), describeResult->Record);
        auto persQueueGroup = describeResult->Record.GetPathDescription().GetPersQueueGroup();

        if (killPqrb)
        {
            Log << TLOG_INFO << "Kill PQRB tablet " << persQueueGroup.GetBalancerTabletID();
            AnnoyingClient->KillTablet(*CleverServer, persQueueGroup.GetBalancerTabletID());
        }

        if (killPq)
        {
            THashSet<ui64> restartedTablets;
            for (const auto& p : persQueueGroup.GetPartitions())
                if (restartedTablets.insert(p.GetTabletId()).second)
                {
                    Log << TLOG_INFO << "Kill PQ tablet " << p.GetTabletId();
                    AnnoyingClient->KillTablet(*CleverServer, p.GetTabletId());
                }
        }

        CleverServer->GetRuntime()->DispatchEvents();
    }

public:
    TString TestCaseName;

    TSimpleSharedPtr<TPortManager> PortManager;
    ui16 Port;
    ui16 GrpcPort;

    THolder<NKikimr::Tests::TServer> CleverServer;
    NKikimr::Tests::TServerSettings ServerSettings;
    NYdbGrpc::TServerOptions GrpcServerOptions;
    THolder<TTempFileHandle> NetDataFile;

    TLog Log = CreateLogBackend("cerr", ELogPriority::TLOG_DEBUG);

    THolder<NKikimr::NPersQueueTests::TFlatMsgBusPQClient> AnnoyingClient;


    static const std::vector<NKikimrServices::EServiceKikimr> LOGGED_SERVICES;
};

} // namespace NPersQueue
