#include <ydb-cpp-sdk/client/topic/client.h>

#include <src/client/topic/impl/topic_impl.h>
#include <src/client/topic/impl/common.h>

#include <src/client/impl/ydb_internal/scheme_helpers/helpers.h>

#include <util/random/random.h>
#include <util/string/cast.h>
#include <util/string/subst.h>

namespace NYdb::NTopic {

class TCommonCodecsProvider {
public:
    TCommonCodecsProvider() {
        TCodecMap::GetTheCodecMap().Set((uint32_t)ECodec::GZIP, std::make_unique<TGzipCodec>());
        TCodecMap::GetTheCodecMap().Set((uint32_t)ECodec::ZSTD, std::make_unique<TZstdCodec>());
    }
};
TCommonCodecsProvider COMMON_CODECS_PROVIDER;

TDescribeTopicResult::TDescribeTopicResult(TStatus&& status, Ydb::Topic::DescribeTopicResult&& result)
    : TStatus(std::move(status))
    , TopicDescription_(std::move(result))
{
}

const TTopicDescription& TDescribeTopicResult::GetTopicDescription() const {
    return TopicDescription_;
}

TDescribeConsumerResult::TDescribeConsumerResult(TStatus&& status, Ydb::Topic::DescribeConsumerResult&& result)
    : TStatus(std::move(status))
    , ConsumerDescription_(std::move(result))
{
}

const TConsumerDescription& TDescribeConsumerResult::GetConsumerDescription() const {
    return ConsumerDescription_;
}

TDescribePartitionResult::TDescribePartitionResult(TStatus&& status, Ydb::Topic::DescribePartitionResult&& result)
    : TStatus(std::move(status))
    , PartitionDescription_(std::move(result))
{
}

const TPartitionDescription& TDescribePartitionResult::GetPartitionDescription() const {
    return PartitionDescription_;
}

TTopicDescription::TTopicDescription(Ydb::Topic::DescribeTopicResult&& result)
    : Proto_(std::move(result))
    , PartitioningSettings_(Proto_.partitioning_settings())
    , RetentionPeriod_(TDuration::Seconds(Proto_.retention_period().seconds()))
    , RetentionStorageMb_(Proto_.retention_storage_mb() > 0 ? std::optional<uint64_t>(Proto_.retention_storage_mb()) : std::nullopt)
    , PartitionWriteSpeedBytesPerSecond_(Proto_.partition_write_speed_bytes_per_second())
    , PartitionWriteBurstBytes_(Proto_.partition_write_burst_bytes())
    , MeteringMode_(TProtoAccessor::FromProto(Proto_.metering_mode()))
    , TopicStats_(Proto_.topic_stats())
{
    Owner_ = Proto_.self().owner();
    CreationTimestamp_ = NScheme::TVirtualTimestamp(Proto_.self().created_at());
    PermissionToSchemeEntry(Proto_.self().permissions(), &Permissions_);
    PermissionToSchemeEntry(Proto_.self().effective_permissions(), &EffectivePermissions_);

    for (const auto& part : Proto_.partitions()) {
        Partitions_.emplace_back(part);
    }
    for (const auto& codec : Proto_.supported_codecs().codecs()) {
        SupportedCodecs_.push_back((ECodec)codec);
    }
    for (const auto& pair : Proto_.attributes()) {
        Attributes_[pair.first] = pair.second;
    }
    for (const auto& consumer : Proto_.consumers()) {
        Consumers_.emplace_back(consumer);
    }
}

TConsumerDescription::TConsumerDescription(Ydb::Topic::DescribeConsumerResult&& result)
    : Proto_(std::move(result))
    , Consumer_(Proto_.consumer())
{
    for (const auto& part : Proto_.partitions()) {
        Partitions_.emplace_back(part);
    }
}

TPartitionDescription::TPartitionDescription(Ydb::Topic::DescribePartitionResult&& result)
    : Proto_(std::move(result))
    , Partition_(Proto_.partition())
{
}

TConsumer::TConsumer(const Ydb::Topic::Consumer& consumer)
    : ConsumerName_(consumer.name())
    , Important_(consumer.important())
    , ReadFrom_(TInstant::Seconds(consumer.read_from().seconds()))
{
    for (const auto& codec : consumer.supported_codecs().codecs()) {
        SupportedCodecs_.push_back((ECodec)codec);
    }
    for (const auto& pair : consumer.attributes()) {
        Attributes_[pair.first] = pair.second;
    }
}

const std::string& TConsumer::GetConsumerName() const {
    return ConsumerName_;
}

bool TConsumer::GetImportant() const {
    return Important_;
}

const TInstant& TConsumer::GetReadFrom() const {
    return ReadFrom_;
}

const std::vector<ECodec>& TConsumer::GetSupportedCodecs() const {
    return SupportedCodecs_;
}

const std::map<std::string, std::string>& TConsumer::GetAttributes() const {
    return Attributes_;
}

const TPartitioningSettings& TTopicDescription::GetPartitioningSettings() const {
    return PartitioningSettings_;
}

uint32_t TTopicDescription::GetTotalPartitionsCount() const {
    return Partitions_.size();
}

const std::vector<TPartitionInfo>& TTopicDescription::GetPartitions() const {
    return Partitions_;
}

const std::vector<TPartitionInfo>& TConsumerDescription::GetPartitions() const {
    return Partitions_;
}

const TPartitionInfo& TPartitionDescription::GetPartition() const {
    return Partition_;
}

const TConsumer& TConsumerDescription::GetConsumer() const {
    return Consumer_;
}

const std::vector<ECodec>& TTopicDescription::GetSupportedCodecs() const {
    return SupportedCodecs_;
}

const TDuration& TTopicDescription::GetRetentionPeriod() const {
    return RetentionPeriod_;
}

std::optional<uint64_t> TTopicDescription::GetRetentionStorageMb() const {
    return RetentionStorageMb_;
}

uint64_t TTopicDescription::GetPartitionWriteSpeedBytesPerSecond() const {
    return PartitionWriteSpeedBytesPerSecond_;
}

uint64_t TTopicDescription::GetPartitionWriteBurstBytes() const {
    return PartitionWriteBurstBytes_;
}

EMeteringMode TTopicDescription::GetMeteringMode() const {
    return MeteringMode_;
}

const std::map<std::string, std::string>& TTopicDescription::GetAttributes() const {
    return Attributes_;
}

const std::vector<TConsumer>& TTopicDescription::GetConsumers() const {
    return Consumers_;
}

void TTopicDescription::SerializeTo(Ydb::Topic::CreateTopicRequest& request) const {
    Y_UNUSED(request);
    Y_ABORT("Not implemented");
}

const Ydb::Topic::DescribeTopicResult& TTopicDescription::GetProto() const {
    return Proto_;
}

const Ydb::Topic::DescribeConsumerResult& TConsumerDescription::GetProto() const {
    return Proto_;
}

const Ydb::Topic::DescribePartitionResult& TPartitionDescription::GetProto() const {
    return Proto_;
}

const std::string& TTopicDescription::GetOwner() const {
    return Owner_;
}

const NScheme::TVirtualTimestamp& TTopicDescription::GetCreationTimestamp() const {
    return CreationTimestamp_;
}

const TTopicStats& TTopicDescription::GetTopicStats() const {
    return TopicStats_;
}

const std::vector<NScheme::TPermissions>& TTopicDescription::GetPermissions() const {
    return Permissions_;
}

const std::vector<NScheme::TPermissions>& TTopicDescription::GetEffectivePermissions() const {
    return EffectivePermissions_;
}

TPartitioningSettings::TPartitioningSettings(const Ydb::Topic::PartitioningSettings& settings)
    : MinActivePartitions_(settings.min_active_partitions())
    , MaxActivePartitions_(settings.max_active_partitions())
    , PartitionCountLimit_(settings.partition_count_limit())
    , AutoPartitioningSettings_(settings.auto_partitioning_settings())
{}

uint64_t TPartitioningSettings::GetMinActivePartitions() const {
    return MinActivePartitions_;
}

uint64_t TPartitioningSettings::GetMaxActivePartitions() const {
    return MaxActivePartitions_;
}

uint64_t TPartitioningSettings::GetPartitionCountLimit() const {
    return PartitionCountLimit_;
}

TAutoPartitioningSettings TPartitioningSettings::GetAutoPartitioningSettings() const {
    return AutoPartitioningSettings_;
}

TAutoPartitioningSettings::TAutoPartitioningSettings(const Ydb::Topic::AutoPartitioningSettings& settings)
    : Strategy_(static_cast<EAutoPartitioningStrategy>(settings.strategy()))
    , StabilizationWindow_(TDuration::Seconds(settings.partition_write_speed().stabilization_window().seconds()))
    , DownUtilizationPercent_(settings.partition_write_speed().down_utilization_percent())
    , UpUtilizationPercent_(settings.partition_write_speed().up_utilization_percent())
{}

EAutoPartitioningStrategy TAutoPartitioningSettings::GetStrategy() const {
    return Strategy_;
}

TDuration TAutoPartitioningSettings::GetStabilizationWindow() const {
    return StabilizationWindow_;
}

uint32_t TAutoPartitioningSettings::GetUpUtilizationPercent() const {
    return UpUtilizationPercent_;
}

uint32_t TAutoPartitioningSettings::GetDownUtilizationPercent() const {
    return DownUtilizationPercent_;
}

TTopicStats::TTopicStats(const Ydb::Topic::DescribeTopicResult::TopicStats& topicStats)
    : StoreSizeBytes_(topicStats.store_size_bytes())
    , MinLastWriteTime_(TInstant::Seconds(topicStats.min_last_write_time().seconds()))
    , MaxWriteTimeLag_(TDuration::Seconds(topicStats.max_write_time_lag().seconds()) + TDuration::MicroSeconds(topicStats.max_write_time_lag().nanos() / 1000))
    , BytesWrittenPerMinute_(topicStats.bytes_written().per_minute())
    , BytesWrittenPerHour_(topicStats.bytes_written().per_hour())
    , BytesWrittenPerDay_(topicStats.bytes_written().per_day())
{
}

uint64_t TTopicStats::GetStoreSizeBytes() const {
    return StoreSizeBytes_;
}

TInstant TTopicStats::GetMinLastWriteTime() const {
    return MinLastWriteTime_;
}

TDuration TTopicStats::GetMaxWriteTimeLag() const {
    return MaxWriteTimeLag_;
}

uint64_t TTopicStats::GetBytesWrittenPerMinute() const {
    return BytesWrittenPerMinute_;
}

uint64_t TTopicStats::GetBytesWrittenPerHour() const {
    return BytesWrittenPerHour_;
}

uint64_t TTopicStats::GetBytesWrittenPerDay() const {
    return BytesWrittenPerDay_;
}


TPartitionStats::TPartitionStats(const Ydb::Topic::PartitionStats& partitionStats)
    : StartOffset_(partitionStats.partition_offsets().start())
    , EndOffset_(partitionStats.partition_offsets().end())
    , StoreSizeBytes_(partitionStats.store_size_bytes())
    , LastWriteTime_(TInstant::Seconds(partitionStats.last_write_time().seconds()))
    , MaxWriteTimeLag_(TDuration::Seconds(partitionStats.max_write_time_lag().seconds()) + TDuration::MicroSeconds(partitionStats.max_write_time_lag().nanos() / 1000))
    , BytesWrittenPerMinute_(partitionStats.bytes_written().per_minute())
    , BytesWrittenPerHour_(partitionStats.bytes_written().per_hour())
    , BytesWrittenPerDay_(partitionStats.bytes_written().per_day())

{}

uint64_t TPartitionStats::GetStartOffset() const {
    return StartOffset_;
}

uint64_t TPartitionStats::GetEndOffset() const {
    return EndOffset_;
}

uint64_t TPartitionStats::GetStoreSizeBytes() const {
    return StoreSizeBytes_;
}

TInstant TPartitionStats::GetLastWriteTime() const {
    return LastWriteTime_;
}

TDuration TPartitionStats::GetMaxWriteTimeLag() const {
    return MaxWriteTimeLag_;
}

uint64_t TPartitionStats::GetBytesWrittenPerMinute() const {
    return BytesWrittenPerMinute_;
}

uint64_t TPartitionStats::GetBytesWrittenPerHour() const {
    return BytesWrittenPerHour_;
}

uint64_t TPartitionStats::GetBytesWrittenPerDay() const {
    return BytesWrittenPerDay_;
}


TPartitionConsumerStats::TPartitionConsumerStats(const Ydb::Topic::DescribeConsumerResult::PartitionConsumerStats& partitionStats)
    : CommittedOffset_(partitionStats.committed_offset())
    , LastReadOffset_(partitionStats.last_read_offset())
    , ReaderName_(partitionStats.reader_name())
    , ReadSessionId_(partitionStats.read_session_id())
{}

uint64_t TPartitionConsumerStats::GetCommittedOffset() const {
    return CommittedOffset_;
}

uint64_t TPartitionConsumerStats::GetLastReadOffset() const {
    return LastReadOffset_;
}

std::string TPartitionConsumerStats::GetReaderName() const {
    return ReaderName_;
}

std::string TPartitionConsumerStats::GetReadSessionId() const {
    return ReadSessionId_;
}

TPartitionLocation::TPartitionLocation(const Ydb::Topic::PartitionLocation& partitionLocation)
    : NodeId_(partitionLocation.node_id())
    , Generation_(partitionLocation.generation())
{
}

int32_t TPartitionLocation::GetNodeId() const {
    return NodeId_;
}

int64_t TPartitionLocation::GetGeneration() const {
    return Generation_;
}

TPartitionInfo::TPartitionInfo(const Ydb::Topic::DescribeTopicResult::PartitionInfo& partitionInfo)
    : PartitionId_(partitionInfo.partition_id())
    , Active_(partitionInfo.active())
    , PartitionStats_()
{
    for (const auto& partId : partitionInfo.child_partition_ids()) {
        ChildPartitionIds_.push_back(partId);
    }

    for (const auto& partId : partitionInfo.parent_partition_ids()) {
        ParentPartitionIds_.push_back(partId);
    }
    if (partitionInfo.has_partition_stats()) {
        PartitionStats_ = TPartitionStats{partitionInfo.partition_stats()};
    }

    if (partitionInfo.has_partition_location()) {
        PartitionLocation_ = TPartitionLocation{partitionInfo.partition_location()};
    }
}

TPartitionInfo::TPartitionInfo(const Ydb::Topic::DescribeConsumerResult::PartitionInfo& partitionInfo)
    : PartitionId_(partitionInfo.partition_id())
    , Active_(partitionInfo.active())
    , PartitionStats_()
{
    for (const auto& partId : partitionInfo.child_partition_ids()) {
        ChildPartitionIds_.push_back(partId);
    }

    for (const auto& partId : partitionInfo.parent_partition_ids()) {
        ParentPartitionIds_.push_back(partId);
    }
    if (partitionInfo.has_partition_stats()) {
        PartitionStats_ = TPartitionStats{partitionInfo.partition_stats()};
        PartitionConsumerStats_ = TPartitionConsumerStats{partitionInfo.partition_consumer_stats()};
    }
    if (partitionInfo.has_partition_location()) {
        PartitionLocation_ = TPartitionLocation{partitionInfo.partition_location()};
    }
}

const std::optional<TPartitionStats>& TPartitionInfo::GetPartitionStats() const {
    return PartitionStats_;
}

const std::optional<TPartitionConsumerStats>& TPartitionInfo::GetPartitionConsumerStats() const {
    return PartitionConsumerStats_;
}

const std::optional<TPartitionLocation>& TPartitionInfo::GetPartitionLocation() const {
    return PartitionLocation_;
}

bool TPartitionInfo::GetActive() const {
    return Active_;
}

uint64_t TPartitionInfo::GetPartitionId() const {
    return PartitionId_;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TTopicClient

TTopicClient::TTopicClient(const TDriver& driver, const TTopicClientSettings& settings)
    : Impl_(std::make_shared<TImpl>(CreateInternalInterface(driver), settings))
{
}

TAsyncStatus TTopicClient::CreateTopic(const std::string& path, const TCreateTopicSettings& settings) {
    return Impl_->CreateTopic(path, settings);
}

TAsyncStatus TTopicClient::AlterTopic(const std::string& path, const TAlterTopicSettings& settings) {
    return Impl_->AlterTopic(path, settings);
}

TAsyncStatus TTopicClient::DropTopic(const std::string& path, const TDropTopicSettings& settings) {
    return Impl_->DropTopic(path, settings);
}

TAsyncDescribeTopicResult TTopicClient::DescribeTopic(const std::string& path, const TDescribeTopicSettings& settings) {
    return Impl_->DescribeTopic(path, settings);
}

TAsyncDescribeConsumerResult TTopicClient::DescribeConsumer(const std::string& path, const std::string& consumer, const TDescribeConsumerSettings& settings) {
    return Impl_->DescribeConsumer(path, consumer, settings);
}

TAsyncDescribePartitionResult TTopicClient::DescribePartition(const std::string& path, int64_t partitionId, const TDescribePartitionSettings& settings) {
    return Impl_->DescribePartition(path, partitionId, settings);
}

std::shared_ptr<IReadSession> TTopicClient::CreateReadSession(const TReadSessionSettings& settings) {
    return Impl_->CreateReadSession(settings);
}

std::shared_ptr<ISimpleBlockingWriteSession> TTopicClient::CreateSimpleBlockingWriteSession(
    const TWriteSessionSettings& settings) {
    return Impl_->CreateSimpleWriteSession(settings);
}

std::shared_ptr<IWriteSession> TTopicClient::CreateWriteSession(const TWriteSessionSettings& settings) {
    return Impl_->CreateWriteSession(settings);
}

TAsyncStatus TTopicClient::CommitOffset(const std::string& path, uint64_t partitionId, const std::string& consumerName, uint64_t offset,
    const TCommitOffsetSettings& settings) {
    return Impl_->CommitOffset(path, partitionId, consumerName, offset, settings);
}

} // namespace NYdb::NTopic
