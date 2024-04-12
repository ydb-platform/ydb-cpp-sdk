#include <src/client/ydb_persqueue_core/ut/ut_utils/ut_utils.h>

<<<<<<<< HEAD:src/client/persqueue_core/ut/retry_policy_ut.cpp
#include <ydb-cpp-sdk/library/threading/future/future.h>
========
#include <src/library/threading/future/future.h>
>>>>>>>> ed2145fb77 (Moved SDK code to src (#149)):src/client/ydb_persqueue_core/ut/retry_policy_ut.cpp
#include <src/library/testing/unittest/registar.h>


using namespace NThreading;
using namespace NKikimr;
using namespace NKikimr::NPersQueueTests;
using namespace NPersQueue;

namespace NYdb::NPersQueue::NTests {

Y_UNIT_TEST_SUITE(RetryPolicy) {
    Y_UNIT_TEST(TWriteSession_TestPolicy) {
        TYdbPqWriterTestHelper helper(TEST_CASE_NAME);
        helper.Write(true);
        helper.Policy->Initialize(); // Thus ignoring possible early retries on "cluster initializing"
        auto doBreakDown = [&] () {
            helper.Policy->ExpectBreakDown();
            NThreading::TPromise<void> retriesPromise = NThreading::NewPromise();
            std::cerr << "WAIT for retries...\n";
            helper.Policy->WaitForRetries(30, retriesPromise);
            std::cerr << "KICK tablets\n";
            helper.Setup->KickTablets();

            auto f1 = helper.Write(false);
            auto f2 = helper.Write();

            auto retriesFuture = retriesPromise.GetFuture();
            retriesFuture.Wait();
            std::cerr << "WAIT for retries done\n";

            NThreading::TPromise<void> repairPromise = NThreading::NewPromise();
            auto repairFuture = repairPromise.GetFuture();
            helper.Policy->WaitForRepair(repairPromise);


            std::cerr << "ALLOW tablets\n";
            helper.Setup->AllowTablets();

            std::cerr << "WAIT for repair\n";
            repairFuture.Wait();
            std::cerr << "REPAIR done\n";
            f1.Wait();
            f2.Wait();
            helper.Write(true);
        };
        doBreakDown();
        doBreakDown();

    }
    Y_UNIT_TEST(TWriteSession_TestBrokenPolicy) {
        TYdbPqWriterTestHelper helper(TEST_CASE_NAME);
        helper.Write();
        helper.Policy->Initialize();
        helper.Policy->ExpectFatalBreakDown();
        helper.EventLoop->AllowStop();
        auto f1 = helper.Write(false);
        helper.Setup->KickTablets();
        helper.Write(false);

        helper.EventLoop->WaitForStop();
        UNIT_ASSERT(!f1.HasValue());
        helper.Setup = nullptr;

    };

    Y_UNIT_TEST(TWriteSession_RetryOnTargetCluster) {
        auto setup1 = std::make_shared<TPersQueueYdbSdkTestSetup>(TEST_CASE_NAME, false);
        SDKTestSetup setup2("RetryOnTargetCluster_Dc2");
        setup1->AddDataCenter("dc2", setup2, false);
        setup1->Start();
        auto retryPolicy = std::make_shared<TYdbPqTestRetryPolicy>();
        auto settings = setup1->GetWriteSessionSettings();
        settings.ClusterDiscoveryMode(EClusterDiscoveryMode::On);
        settings.PreferredCluster("dc2");
        settings.AllowFallbackToOtherClusters(false);
        settings.RetryPolicy(retryPolicy);

        retryPolicy->Initialize();
        retryPolicy->ExpectBreakDown();

        auto& client = setup1->GetPersQueueClient();
        std::cerr << "=== Create write session \n";
        auto writer = client.CreateWriteSession(settings);

        NThreading::TPromise<void> retriesPromise = NThreading::NewPromise();
        auto retriesFuture = retriesPromise.GetFuture();
        retryPolicy->WaitForRetries(3, retriesPromise);
        std::cerr << "=== Wait retries\n";
        retriesFuture.Wait();

        std::cerr << "=== Enable dc2\n";
        setup1->EnableDataCenter("dc2");

        NThreading::TPromise<void> repairPromise = NThreading::NewPromise();
        auto repairFuture = repairPromise.GetFuture();
        retryPolicy->WaitForRepair(repairPromise);
        std::cerr << "=== Wait for repair\n";
        repairFuture.Wait();
        std::cerr << "=== Close writer\n";
        writer->Close();
    }

    Y_UNIT_TEST(TWriteSession_SwitchBackToLocalCluster) {
        std::cerr << "====Start test\n";

        auto setup1 = std::make_shared<TPersQueueYdbSdkTestSetup>(TEST_CASE_NAME, false);
        SDKTestSetup setup2("SwitchBackToLocalCluster", false);
        setup2.SetSingleDataCenter("dc2");
        setup2.AddDataCenter("dc1", *setup1, true);
        setup1->AddDataCenter("dc2", setup2, true);
        setup1->Start();
        setup2.Start(false);
        std::cerr << "=== Start session 1\n";
        auto helper = MakeHolder<TYdbPqWriterTestHelper>("", nullptr, std::string(), setup1);
        helper->Write(true);
        auto retryPolicy = helper->Policy;
        retryPolicy->Initialize();

        auto waitForReconnect = [&](bool enable) {
            std::cerr << "=== Expect breakdown\n";
            retryPolicy->ExpectBreakDown();

            NThreading::TPromise<void> retriesPromise = NThreading::NewPromise();
            auto retriesFuture = retriesPromise.GetFuture();
            retryPolicy->WaitForRetries(1, retriesPromise);

            NThreading::TPromise<void> repairPromise = NThreading::NewPromise();
            auto repairFuture = repairPromise.GetFuture();
            retryPolicy->WaitForRepair(repairPromise);

            if (enable) {
                std::cerr << "===Enabled DC1\n";
                setup1->EnableDataCenter("dc1");
                setup2.EnableDataCenter("dc1");
            } else {
                std::cerr << "===Disabled DC1\n";
                setup1->DisableDataCenter("dc1");
                setup2.DisableDataCenter("dc1");
            }
            Sleep(TDuration::Seconds(5));

            retriesFuture.Wait();
            repairFuture.Wait();
        };
        std::cerr << "===Wait for 1st reconnect\n";
        waitForReconnect(false);
        std::cerr << "===Wait for 2nd reconnect\n";
        waitForReconnect(true);
    }

    Y_UNIT_TEST(TWriteSession_SeqNoShift) {
        auto setup1 = std::make_shared<TPersQueueYdbSdkTestSetup>(TEST_CASE_NAME, false, TTestServer::LOGGED_SERVICES, NActors::NLog::PRI_TRACE);
        SDKTestSetup setup2("SeqNoShift_Dc2", false, TTestServer::LOGGED_SERVICES, NActors::NLog::PRI_TRACE);
        setup2.SetSingleDataCenter("dc2");
        setup2.AddDataCenter("dc1", *setup1, true);
        setup2.Start(true, false);
        setup1->AddDataCenter("dc2", setup2, true);
        setup1->Start(true, false);

        std::string sourceId1 = SDKTestSetup::GetTestMessageGroupId() + "1";
        std::string sourceId2 = SDKTestSetup::GetTestMessageGroupId() + "2";
        auto writer1 = MakeHolder<TYdbPqWriterTestHelper>("", nullptr, "dc1", setup1, sourceId1 , true);
        auto writer2 = MakeHolder<TYdbPqWriterTestHelper>("", nullptr, "dc1", setup1, sourceId2, true);

        auto settings = setup1->GetWriteSessionSettings();
        auto& client = setup1->GetPersQueueClient();

        //! Fill data in dc1 1 with SeqNo = 1..10 for 2 different SrcId
        std::cerr << "===Write 10 messages into every writer\n";
        for (auto i = 0; i != 10; i++) {
            writer1->Write(true); // 1
            writer2->Write(true); // 1
        }
        std::cerr << "===Messages were written\n";

        std::cerr << "===Disable dc1\n";
        //! Leave only dc2 available
        writer1->Policy->ExpectBreakDown();
        writer2->Policy->ExpectBreakDown();
        setup1->DisableDataCenter("dc1");
        writer1->Policy->WaitForRetriesSync(1);
        writer2->Policy->WaitForRetriesSync(1);

        std::cerr << "===Recreate writers\n";

        //! Re-create writers, kill previous sessions. New sessions will connect to dc2.
        writer1 = MakeHolder<TYdbPqWriterTestHelper>("", nullptr, std::string(), setup1, sourceId1, true);
        writer2 = MakeHolder<TYdbPqWriterTestHelper>("", nullptr, std::string(), setup1, sourceId2, true);

        //! Write some data and await confirmation - just to ensure sessions are started.
        std::cerr << "===Write one message into every writer\n";
        writer1->Write(true);
        writer2->Write(true);
        std::cerr << "===Messages were written\n";

        //! Leave no available DCs
        writer1->Policy->ExpectBreakDown();
        writer2->Policy->ExpectBreakDown();
        writer1->Policy->Initialize();
        writer2->Policy->Initialize();

        std::cerr << "===Disable dc2\n";
        setup1->DisableDataCenter("dc2");
        std::cerr << "===Wait for retries after initial dc2 shutdown\n";
        writer1->Policy->WaitForRetriesSync(1);
        writer2->Policy->WaitForRetriesSync(1);

        //! Put some data inflight. It cannot be written now, but SeqNo will be assigned.
        std::cerr << "===Write four async message into every writer\n";
        for (auto i = 0; i != 3; i++) {
            writer1->Write(false);
            writer2->Write(false);
        }
        auto f1 = writer1->Write(false);
        auto f2 = writer2->Write(false);

        //! Enable DC1. Now writers gonna write collected data to DC1 having LastSeqNo = 10
        //! (because of data written in the very beginning), and inflight data has SeqNo assigned = 2..5,
        //! so the SeqNo shift takes place.
        std::cerr << "===Enable dc2\n";
        setup1->EnableDataCenter("dc1");

        std::cerr << "===Wait for writes to complete\n";
        f1.Wait();
        f2.Wait();
        std::cerr << "===Messages were written\n";

        //! Writer1 is not used any more.
        writer1->EventLoop->AllowStop();
        writer1 = nullptr;

        std::cerr << "===Writer 1 closed\n";
        writer2->Policy->ExpectBreakDown();
        writer2->Policy->Initialize();

        //! For the second writer, do switchback to dc2.
        std::cerr << "===Disable dc1\n";
        setup1->DisableDataCenter("dc1");
        std::cerr << "===Wait for retries after dc1 shutdown\n";
        writer2->Policy->WaitForRetriesSync(1);

        //! Put some data inflight again;
        std::cerr << "===Write four async messages into writer2\n";
        for (auto i = 0; i != 3; i++) {
            writer2->Write(false);
        }
        f2 = writer2->Write(false);

        std::cerr << "===Enable dc2\n";
        setup1->EnableDataCenter("dc2");

        f2.Wait();
        std::cerr << "===Messages were written\n";

        writer2->EventLoop->AllowStop();
        writer2->Policy->ExpectBreakDown();
        writer2 = nullptr;

        std::cerr << "===Enable dc1\n";
        setup1->EnableDataCenter("dc1");
        auto CheckSeqNo = [&] (const std::string& dcName, ui64 expectedSeqNo) {
            settings.PreferredCluster(dcName);
            settings.AllowFallbackToOtherClusters(false);
            settings.RetryPolicy(nullptr); //switch to default policy;
            auto writer = client.CreateWriteSession(settings);
            auto seqNo = writer->GetInitSeqNo().GetValueSync();
            UNIT_ASSERT_VALUES_EQUAL(seqNo, expectedSeqNo);
            writer->Close(TDuration::Zero());
        };

        //!check SeqNo in both DC. For writer1 We expect 14 messages in DC1
        //! (10 written initially + 4 written after reconnect) and 1 message in DC2 (only initial message).
        settings.MessageGroupId(sourceId1);
        std::cerr << "===Check SeqNo writer1, dc2\n";
        CheckSeqNo("dc2", 1);
        std::cerr << "===Check SeqNo writer1, dc1\n";
        CheckSeqNo("dc1", 14);

        //! Check SeqNo for writer 2; Expect to have 6 messages on DC2 with MaxSeqNo = 6;
        settings.MessageGroupId(sourceId2);
        std::cerr << "===Check SeqNo writer2 dc1\n";
        CheckSeqNo("dc1", 14);
        //! DC2 has no shift in SeqNo since 5 messages were written to dc 1.
        std::cerr << "===Check SeqNo writer2 dc2\n";
        CheckSeqNo("dc2", 9);


        auto readSession = client.CreateReadSession(setup1->GetReadSessionSettings());

        bool stop = false;
        THashMap<std::string, ui64> seqNoByClusterSrc1 = {
                {"dc1", 0},
                {"dc2", 0}
        };
        auto SeqNoByClusterSrc2 = seqNoByClusterSrc1;

        THashMap<std::string, ui64> MsgCountByClusterSrc1 = {
                {"dc1", 14},
                {"dc2", 1}
        };
        THashMap<std::string, ui64> MsgCountByClusterSrc2 = {
                {"dc1", 14},
                {"dc2", 5}
        };
        ui32 clustersPendingSrc1 = 2;
        ui32 clustersPendingSrc2 = 2;

        while (!stop && (clustersPendingSrc2 || clustersPendingSrc1)) {
            std::cerr << "===Get event on client\n";
            auto event = *readSession->GetEvent(true);
            std::visit(TOverloaded {
                    [&](TReadSessionEvent::TDataReceivedEvent& event) {
                        std::cerr << "===Data event\n";
                        auto& clusterName = event.GetPartitionStream()->GetCluster();
                        for (auto& message: event.GetMessages()) {
                            std::string sourceId = message.GetMessageGroupId();
                            ui32 seqNo = message.GetSeqNo();
                            if (sourceId == sourceId1) {
                                UNIT_ASSERT_VALUES_EQUAL(seqNo, seqNoByClusterSrc1[clusterName] + 1);
                                seqNoByClusterSrc1[clusterName]++;
                                auto& msgRemaining = MsgCountByClusterSrc1[clusterName];
                                UNIT_ASSERT(msgRemaining > 0);
                                msgRemaining--;
                                if (!msgRemaining)
                                    clustersPendingSrc1--;
                            } else {
                                UNIT_ASSERT_VALUES_EQUAL(sourceId, sourceId2);
                                auto& prevSeqNo = SeqNoByClusterSrc2[clusterName];
                                if (clusterName == "dc1") {
                                    UNIT_ASSERT_VALUES_EQUAL(seqNo, prevSeqNo + 1);
                                    prevSeqNo++;
                                } else {
                                    UNIT_ASSERT_VALUES_EQUAL(clusterName, "dc2");
                                    if (prevSeqNo == 0) {
                                        UNIT_ASSERT_VALUES_EQUAL(seqNo, 1);
                                    } else if (prevSeqNo == 1) {
                                        UNIT_ASSERT_VALUES_EQUAL(seqNo, 6);
                                    } else {
                                        UNIT_ASSERT_VALUES_EQUAL(seqNo, prevSeqNo + 1);
                                    }
                                    prevSeqNo = seqNo;
                                }
                                auto& msgRemaining = MsgCountByClusterSrc2[clusterName];
                                UNIT_ASSERT(msgRemaining > 0);
                                msgRemaining--;
                                if (!msgRemaining)
                                    clustersPendingSrc2--;
                            }
                            message.Commit();
                        }
                    },
                    [&](TReadSessionEvent::TCommitAcknowledgementEvent&) {
                    },
                    [&](TReadSessionEvent::TCreatePartitionStreamEvent& event) {
                        event.Confirm();
                    },
                    [&](TReadSessionEvent::TDestroyPartitionStreamEvent& event) {
                        event.Confirm();
                    },
                    [&](TReadSessionEvent::TPartitionStreamStatusEvent&) {
                        std::cerr << "===Status event\n";
                        UNIT_FAIL("Test does not support lock sessions yet");
                    },
                    [&](TReadSessionEvent::TPartitionStreamClosedEvent&) {
                        std::cerr << "===Stream closed event\n";
                        UNIT_FAIL("Test does not support lock sessions yet");
                    },
                    [&](TSessionClosedEvent& event) {
                        std::cerr << "===Got close event: " << event.DebugString();
                        stop = true;
                    }

            }, event);
        }
        UNIT_ASSERT_VALUES_EQUAL(clustersPendingSrc1 || clustersPendingSrc2, 0);
    }
    Y_UNIT_TEST(RetryWithBatching) {
        auto setup = std::make_shared<TPersQueueYdbSdkTestSetup>(TEST_CASE_NAME);
        auto settings = setup->GetWriteSessionSettings();
        auto retryPolicy = std::make_shared<TYdbPqTestRetryPolicy>();
        settings.BatchFlushInterval(TDuration::Seconds(1000)); // Batch on size, not on time.
        settings.BatchFlushSizeBytes(100);
        settings.RetryPolicy(retryPolicy);
        auto& client = setup->GetPersQueueClient();
        auto writer = client.CreateWriteSession(settings);
        auto event = *writer->GetEvent(true);
        std::cerr << NYdb::NPersQueue::DebugString(event) << "\n";
        UNIT_ASSERT(std::holds_alternative<TWriteSessionEvent::TReadyToAcceptEvent>(event));
        auto continueToken = std::move(std::get<TWriteSessionEvent::TReadyToAcceptEvent>(event).ContinuationToken);
        std::string message = "1234567890";
        ui64 seqNo = 0;
        setup->KickTablets();
        writer->Write(std::move(continueToken), message, ++seqNo);
        retryPolicy->ExpectBreakDown();
        retryPolicy->WaitForRetriesSync(3);
        while (seqNo < 10) {
            auto event = *writer->GetEvent(true);
            std::cerr << NYdb::NPersQueue::DebugString(event) << "\n";
            UNIT_ASSERT(std::holds_alternative<TWriteSessionEvent::TReadyToAcceptEvent>(event));
            writer->Write(
                    std::move(std::get<TWriteSessionEvent::TReadyToAcceptEvent>(event).ContinuationToken),
                    message, ++seqNo
            );
        }

        setup->AllowTablets();
        retryPolicy->WaitForRepairSync();
        WaitMessagesAcked(writer, 1, seqNo);
    }
};
}; //NYdb::NPersQueue::NTests
