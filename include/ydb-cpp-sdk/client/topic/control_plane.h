#pragma once

#include "codecs.h"

#include <ydb-cpp-sdk/client/scheme/scheme.h>

#include <src/api/grpc/ydb_topic_v1.grpc.pb.h>

#include <ydb-cpp-sdk/util/datetime/base.h>

#include <limits>

namespace NYdb {
    class TProtoAccessor;

    namespace NScheme {
        struct TPermissions;
    }
}

namespace NYdb::NTopic {
    
enum class EMeteringMode : uint32_t {
    Unspecified = 0,
    ReservedCapacity = 1,
    RequestUnits = 2,

    Unknown = std::numeric_limits<int>::max(),
};

enum class EAutoscalingStrategy: uint32_t {
    Unspecified = 0,
    Disabled = 1,
    ScaleUp = 2,
    ScaleUpAndDown = 3,
};

class TConsumer {
public:
    TConsumer(const Ydb::Topic::Consumer&);

    const std::string& GetConsumerName() const;
    bool GetImportant() const;
    const TInstant& GetReadFrom() const;
    const std::vector<ECodec>& GetSupportedCodecs() const;
    const std::map<std::string, std::string>& GetAttributes() const;

private:
    std::string ConsumerName_;
    bool Important_;
    TInstant ReadFrom_;
    std::map<std::string, std::string> Attributes_;
    std::vector<ECodec> SupportedCodecs_;

};


class TTopicStats {
public:
    TTopicStats(const Ydb::Topic::DescribeTopicResult::TopicStats& topicStats);

    uint64_t GetStoreSizeBytes() const;
    TDuration GetMaxWriteTimeLag() const;
    TInstant GetMinLastWriteTime() const;
    uint64_t GetBytesWrittenPerMinute() const;
    uint64_t GetBytesWrittenPerHour() const;
    uint64_t GetBytesWrittenPerDay() const;

private:
    uint64_t StoreSizeBytes_;
    TInstant MinLastWriteTime_;
    TDuration MaxWriteTimeLag_;
    uint64_t BytesWrittenPerMinute_;
    uint64_t BytesWrittenPerHour_;
    uint64_t BytesWrittenPerDay_;
};


class TPartitionStats {
public:
    TPartitionStats(const Ydb::Topic::PartitionStats& partitionStats);

    uint64_t GetStartOffset() const;
    uint64_t GetEndOffset() const;
    uint64_t GetStoreSizeBytes() const;
    TDuration GetMaxWriteTimeLag() const;
    TInstant GetLastWriteTime() const;
    uint64_t GetBytesWrittenPerMinute() const;
    uint64_t GetBytesWrittenPerHour() const;
    uint64_t GetBytesWrittenPerDay() const;

private:
    uint64_t StartOffset_;
    uint64_t EndOffset_;
    uint64_t StoreSizeBytes_;
    TInstant LastWriteTime_;
    TDuration MaxWriteTimeLag_;
    uint64_t BytesWrittenPerMinute_;
    uint64_t BytesWrittenPerHour_;
    uint64_t BytesWrittenPerDay_;
};

class TPartitionConsumerStats {
public:
    TPartitionConsumerStats(const Ydb::Topic::DescribeConsumerResult::PartitionConsumerStats& partitionStats);
    uint64_t GetCommittedOffset() const;
    uint64_t GetLastReadOffset() const;
    std::string GetReaderName() const;
    std::string GetReadSessionId() const;

private:
    uint64_t CommittedOffset_;
    int64_t LastReadOffset_;
    std::string ReaderName_;
    std::string ReadSessionId_;
};

// Topic partition location
class TPartitionLocation {
public:
    TPartitionLocation(const Ydb::Topic::PartitionLocation& partitionLocation);
    int32_t GetNodeId() const;
    int64_t GetGeneration() const;

private:
    // Node identificator.
    int32_t NodeId_ = 1;

    // Partition generation.
    int64_t Generation_ = 2;
};

class TPartitionInfo {
public:
    TPartitionInfo(const Ydb::Topic::DescribeTopicResult::PartitionInfo& partitionInfo);
    TPartitionInfo(const Ydb::Topic::DescribeConsumerResult::PartitionInfo& partitionInfo);

    uint64_t GetPartitionId() const;
    bool GetActive() const;
    const std::vector<uint64_t> GetChildPartitionIds() const;
    const std::vector<uint64_t> GetParentPartitionIds() const;

    const std::optional<TPartitionStats>& GetPartitionStats() const;
    const std::optional<TPartitionConsumerStats>& GetPartitionConsumerStats() const;
    const std::optional<TPartitionLocation>& GetPartitionLocation() const;

private:
    uint64_t PartitionId_;
    bool Active_;
    std::vector<uint64_t> ChildPartitionIds_;
    std::vector<uint64_t> ParentPartitionIds_;
    std::optional<TPartitionStats> PartitionStats_;
    std::optional<TPartitionConsumerStats> PartitionConsumerStats_;
    std::optional<TPartitionLocation> PartitionLocation_;
};

struct TAlterPartitioningSettings;
struct TAlterTopicSettings;

struct TAutoscalingSettings {
friend struct TAutoscalingSettingsBuilder;
public:
    TAutoscalingSettings()
        : Strategy_(EAutoscalingStrategy::Disabled)
        , ThresholdTime_(TDuration::Seconds(0))
        , ScaleDownThresholdPercent_(0)
        , ScaleUpThresholdPercent_(0) {
    }
    TAutoscalingSettings(const Ydb::Topic::AutoscalingSettings& settings);
    TAutoscalingSettings(EAutoscalingStrategy strategy, TDuration thresholdTime, uint64_t scaleUpThresholdPercent, uint64_t scaleDownThresholdPercent)
        : Strategy_(strategy)
        , ThresholdTime_(thresholdTime)
        , ScaleDownThresholdPercent_(scaleDownThresholdPercent)
        , ScaleUpThresholdPercent_(scaleUpThresholdPercent) {}

    EAutoscalingStrategy GetStrategy() const;
    TDuration GetThresholdTime() const;
    uint32_t GetScaleDownThresholdPercent() const;
    uint32_t GetScaleUpThresholdPercent() const;
private:
    EAutoscalingStrategy Strategy_;
    TDuration ThresholdTime_;
    uint32_t ScaleDownThresholdPercent_;
    uint32_t ScaleUpThresholdPercent_;
};

struct TAlterAutoscalingSettings {
    using TSelf = TAlterAutoscalingSettings;
    public:
        TAlterAutoscalingSettings(TAlterPartitioningSettings& parent): Parent_(parent) {}

    FLUENT_SETTING_OPTIONAL(EAutoscalingStrategy, Strategy);
    FLUENT_SETTING_OPTIONAL(TDuration, ThresholdTime);
    FLUENT_SETTING_OPTIONAL(uint64_t, ScaleUpThresholdPercent);
    FLUENT_SETTING_OPTIONAL(uint64_t, ScaleDownThresholdPercent);

    TAlterPartitioningSettings& EndAlterAutoscalingSettings() { return Parent_; };

    private:
        TAlterPartitioningSettings& Parent_;
};

class TPartitioningSettings {
    using TSelf = TPartitioningSettings;
    friend struct TPartitioningSettingsBuilder;
public:
    TPartitioningSettings() : MinActivePartitions_(0), MaxActivePartitions_(0), PartitionCountLimit_(0), AutoscalingSettings_(){}
    TPartitioningSettings(const Ydb::Topic::PartitioningSettings& settings);
    TPartitioningSettings(uint64_t minActivePartitions, uint64_t maxActivePartitions, TAutoscalingSettings autoscalingSettings = {})
        : MinActivePartitions_(minActivePartitions)
        , MaxActivePartitions_(maxActivePartitions)
        , PartitionCountLimit_(0)
        , AutoscalingSettings_(autoscalingSettings)
    {
    }

    uint64_t GetMinActivePartitions() const;
    uint64_t GetMaxActivePartitions() const;
    uint64_t GetPartitionCountLimit() const;
    TAutoscalingSettings GetAutoscalingSettings() const;
private:
    uint64_t MinActivePartitions_;
    uint64_t MaxActivePartitions_;
    uint64_t PartitionCountLimit_;
    TAutoscalingSettings AutoscalingSettings_;
};

struct TAlterTopicSettings;

struct TAlterPartitioningSettings {
    using TSelf = TAlterPartitioningSettings;
public:
    TAlterPartitioningSettings(TAlterTopicSettings& parent): Parent_(parent) {}

    FLUENT_SETTING_OPTIONAL(uint64_t, MinActivePartitions);
    FLUENT_SETTING_OPTIONAL(uint64_t, MaxActivePartitions);

    TAlterTopicSettings& EndAlterTopicPartitioningSettings() { return Parent_; };

    TAlterAutoscalingSettings& BeginAlterAutoscalingSettings() {
        AutoscalingSettings_.emplace(*this);
        return *AutoscalingSettings_;
    }

    std::optional<TAlterAutoscalingSettings> AutoscalingSettings_;

private:
    TAlterTopicSettings& Parent_;
};

class TTopicDescription {
    friend class NYdb::TProtoAccessor;

public:
    TTopicDescription(Ydb::Topic::DescribeTopicResult&& desc);

    const std::string& GetOwner() const;

    const NScheme::TVirtualTimestamp& GetCreationTimestamp() const;

    const std::vector<NScheme::TPermissions>& GetPermissions() const;

    const std::vector<NScheme::TPermissions>& GetEffectivePermissions() const;

    const TPartitioningSettings& GetPartitioningSettings() const;

    uint32_t GetTotalPartitionsCount() const;

    const std::vector<TPartitionInfo>& GetPartitions() const;

    const std::vector<ECodec>& GetSupportedCodecs() const;

    const TDuration& GetRetentionPeriod() const;

    std::optional<uint64_t> GetRetentionStorageMb() const;

    uint64_t GetPartitionWriteSpeedBytesPerSecond() const;

    uint64_t GetPartitionWriteBurstBytes() const;

    const std::map<std::string, std::string>& GetAttributes() const;

    const std::vector<TConsumer>& GetConsumers() const;

    EMeteringMode GetMeteringMode() const;

    const TTopicStats& GetTopicStats() const;

    void SerializeTo(Ydb::Topic::CreateTopicRequest& request) const;
private:

    const Ydb::Topic::DescribeTopicResult& GetProto() const;

    const Ydb::Topic::DescribeTopicResult Proto_;
    std::vector<TPartitionInfo> Partitions_;
    std::vector<ECodec> SupportedCodecs_;
    TPartitioningSettings PartitioningSettings_;
    TDuration RetentionPeriod_;
    std::optional<uint64_t> RetentionStorageMb_;
    uint64_t PartitionWriteSpeedBytesPerSecond_;
    uint64_t PartitionWriteBurstBytes_;
    EMeteringMode MeteringMode_;
    std::map<std::string, std::string> Attributes_;
    std::vector<TConsumer> Consumers_;

    TTopicStats TopicStats_;

    std::string Owner_;
    NScheme::TVirtualTimestamp CreationTimestamp_;
    std::vector<NScheme::TPermissions> Permissions_;
    std::vector<NScheme::TPermissions> EffectivePermissions_;
};

class TConsumerDescription {
    friend class NYdb::TProtoAccessor;

public:
    TConsumerDescription(Ydb::Topic::DescribeConsumerResult&& desc);

    const std::vector<TPartitionInfo>& GetPartitions() const;

    const TConsumer& GetConsumer() const;

private:

    const Ydb::Topic::DescribeConsumerResult& GetProto() const;


    const Ydb::Topic::DescribeConsumerResult Proto_;
    std::vector<TPartitionInfo> Partitions_;
    TConsumer Consumer_;
};

class TPartitionDescription {
    friend class NYdb::TProtoAccessor;

public:
    TPartitionDescription(Ydb::Topic::DescribePartitionResult&& desc);

    const TPartitionInfo& GetPartition() const;
private:
    const Ydb::Topic::DescribePartitionResult& GetProto() const;

    const Ydb::Topic::DescribePartitionResult Proto_;
    TPartitionInfo Partition_;
};

// Result for describe topic request.
struct TDescribeTopicResult : public TStatus {
    friend class NYdb::TProtoAccessor;

    TDescribeTopicResult(TStatus&& status, Ydb::Topic::DescribeTopicResult&& result);

    const TTopicDescription& GetTopicDescription() const;

private:
    TTopicDescription TopicDescription_;
};

// Result for describe consumer request.
struct TDescribeConsumerResult : public TStatus {
    friend class NYdb::TProtoAccessor;

    TDescribeConsumerResult(TStatus&& status, Ydb::Topic::DescribeConsumerResult&& result);

    const TConsumerDescription& GetConsumerDescription() const;

private:
    TConsumerDescription ConsumerDescription_;
};

// Result for describe partition request.
struct TDescribePartitionResult: public TStatus {
    friend class NYdb::TProtoAccessor;

    TDescribePartitionResult(TStatus&& status, Ydb::Topic::DescribePartitionResult&& result);

    const TPartitionDescription& GetPartitionDescription() const;

private:
    TPartitionDescription PartitionDescription_;
};

using TAsyncDescribeTopicResult = NThreading::TFuture<TDescribeTopicResult>;
using TAsyncDescribeConsumerResult = NThreading::TFuture<TDescribeConsumerResult>;
using TAsyncDescribePartitionResult = NThreading::TFuture<TDescribePartitionResult>;

template <class TSettings>
class TAlterAttributesBuilderImpl {
public:
    TAlterAttributesBuilderImpl(TSettings& parent)
    : Parent_(parent)
    { }

    TAlterAttributesBuilderImpl& Alter(const std::string& key, const std::string& value) {
        Parent_.AlterAttributes_[key] = value;
        return *this;
    }

    TAlterAttributesBuilderImpl& Add(const std::string& key, const std::string& value) {
        return Alter(key, value);
    }

    TAlterAttributesBuilderImpl& Drop(const std::string& key) {
        return Alter(key, "");
    }

    TSettings& EndAlterAttributes() { return Parent_; }

private:
    TSettings& Parent_;
};

struct TAlterConsumerSettings;
struct TAlterTopicSettings;

using TAlterConsumerAttributesBuilder = TAlterAttributesBuilderImpl<TAlterConsumerSettings>;
using TAlterTopicAttributesBuilder = TAlterAttributesBuilderImpl<TAlterTopicSettings>;

template<class TSettings>
struct TConsumerSettings {
    using TSelf = TConsumerSettings;

    using TAttributes = std::map<std::string, std::string>;

    TConsumerSettings(TSettings& parent): Parent_(parent) {}
    TConsumerSettings(TSettings& parent, const std::string& name) : ConsumerName_(name), Parent_(parent) {}

    FLUENT_SETTING(std::string, ConsumerName);
    FLUENT_SETTING_DEFAULT(bool, Important, false);
    FLUENT_SETTING_DEFAULT(TInstant, ReadFrom, TInstant::Zero());

    FLUENT_SETTING_VECTOR(ECodec, SupportedCodecs);

    FLUENT_SETTING(TAttributes, Attributes);

    TConsumerSettings& AddAttribute(const std::string& key, const std::string& value) {
        Attributes_[key] = value;
        return *this;
    }

    TConsumerSettings& SetAttributes(std::map<std::string, std::string>&& attributes) {
        Attributes_ = std::move(attributes);
        return *this;
    }

    TConsumerSettings& SetAttributes(const std::map<std::string, std::string>& attributes) {
        Attributes_ = attributes;
        return *this;
    }

    TConsumerSettings& SetSupportedCodecs(std::vector<ECodec>&& codecs) {
        SupportedCodecs_ = std::move(codecs);
        return *this;
    }

    TConsumerSettings& SetSupportedCodecs(const std::vector<ECodec>& codecs) {
        SupportedCodecs_ = codecs;
        return *this;
    }

    TSettings& EndAddConsumer() { return Parent_; };

private:
    TSettings& Parent_;
};

struct TAlterConsumerSettings {
    using TSelf = TAlterConsumerSettings;

    using TAlterAttributes = std::map<std::string, std::string>;

    TAlterConsumerSettings(TAlterTopicSettings& parent): Parent_(parent) {}
    TAlterConsumerSettings(TAlterTopicSettings& parent, const std::string& name) : ConsumerName_(name), Parent_(parent) {}

    FLUENT_SETTING(std::string, ConsumerName);
    FLUENT_SETTING_OPTIONAL(bool, SetImportant);
    FLUENT_SETTING_OPTIONAL(TInstant, SetReadFrom);

    FLUENT_SETTING_OPTIONAL_VECTOR(ECodec, SetSupportedCodecs);

    FLUENT_SETTING(TAlterAttributes, AlterAttributes);

    TAlterConsumerAttributesBuilder BeginAlterAttributes() {
        return TAlterConsumerAttributesBuilder(*this);
    }

    TAlterConsumerSettings& SetSupportedCodecs(std::vector<ECodec>&& codecs) {
        SetSupportedCodecs_ = std::move(codecs);
        return *this;
    }

    TAlterConsumerSettings& SetSupportedCodecs(const std::vector<ECodec>& codecs) {
        SetSupportedCodecs_ = codecs;
        return *this;
    }

    TAlterTopicSettings& EndAlterConsumer() { return Parent_; };

private:
    TAlterTopicSettings& Parent_;
};

struct TPartitioningSettingsBuilder;
struct TCreateTopicSettings : public TOperationRequestSettings<TCreateTopicSettings> {

    using TSelf = TCreateTopicSettings;
    using TAttributes = std::map<std::string, std::string>;

    FLUENT_SETTING(TPartitioningSettings, PartitioningSettings);

    FLUENT_SETTING_DEFAULT(TDuration, RetentionPeriod, TDuration::Hours(24));

    FLUENT_SETTING_VECTOR(ECodec, SupportedCodecs);

    FLUENT_SETTING_DEFAULT(uint64_t, RetentionStorageMb, 0);
    FLUENT_SETTING_DEFAULT(EMeteringMode, MeteringMode, EMeteringMode::Unspecified);

    FLUENT_SETTING_DEFAULT(uint64_t, PartitionWriteSpeedBytesPerSecond, 0);
    FLUENT_SETTING_DEFAULT(uint64_t, PartitionWriteBurstBytes, 0);

    FLUENT_SETTING_VECTOR(TConsumerSettings<TCreateTopicSettings>, Consumers);

    FLUENT_SETTING(TAttributes, Attributes);

    TCreateTopicSettings& SetSupportedCodecs(std::vector<ECodec>&& codecs) {
        SupportedCodecs_ = std::move(codecs);
        return *this;
    }

    TCreateTopicSettings& SetSupportedCodecs(const std::vector<ECodec>& codecs) {
        SupportedCodecs_ = codecs;
        return *this;
    }

    TConsumerSettings<TCreateTopicSettings>& BeginAddConsumer() {
        Consumers_.push_back({*this});
        return Consumers_.back();
    }

    TConsumerSettings<TCreateTopicSettings>& BeginAddConsumer(const std::string& name) {
        Consumers_.push_back({*this, name});
        return Consumers_.back();
    }

    TCreateTopicSettings& AddAttribute(const std::string& key, const std::string& value) {
        Attributes_[key] = value;
        return *this;
    }

    TCreateTopicSettings& SetAttributes(std::map<std::string, std::string>&& attributes) {
        Attributes_ = std::move(attributes);
        return *this;
    }

    TCreateTopicSettings& SetAttributes(const std::map<std::string, std::string>& attributes) {
        Attributes_ = attributes;
        return *this;
    }

    TCreateTopicSettings& PartitioningSettings(uint64_t minActivePartitions, uint64_t maxActivePartitions, TAutoscalingSettings autoscalingSettings = {}) {
        PartitioningSettings_ = TPartitioningSettings(minActivePartitions, maxActivePartitions, autoscalingSettings);
        return *this;
    }

    TPartitioningSettingsBuilder BeginConfigurePartitioningSettings();
};

struct TAutoscalingSettingsBuilder {
    using TSelf = TAutoscalingSettingsBuilder;
public:
    TAutoscalingSettingsBuilder(TPartitioningSettingsBuilder& parent, TAutoscalingSettings& settings): Parent_(parent), Settings_(settings) {}

    TSelf Strategy(EAutoscalingStrategy value) {
        Settings_.Strategy_ = value;
        return *this;
    }

    TSelf ThresholdTime(TDuration value) {
        Settings_.ThresholdTime_ = value;
        return *this;
    }

    TSelf ScaleDownThresholdPercent(uint32_t value) {
        Settings_.ScaleDownThresholdPercent_ = value;
        return *this;
    }

    TSelf ScaleUpThresholdPercent(uint32_t value) {
        Settings_.ScaleUpThresholdPercent_ = value;
        return *this;
    }

    TPartitioningSettingsBuilder& EndConfigureAutoscalingSettings() {
        return Parent_;
    }

private:
    TPartitioningSettingsBuilder& Parent_;
    TAutoscalingSettings& Settings_;
};

struct TPartitioningSettingsBuilder {
    using TSelf = TPartitioningSettingsBuilder;
public:
    TPartitioningSettingsBuilder(TCreateTopicSettings& parent): Parent_(parent) {}

    TSelf MinActivePartitions(uint64_t value) {
        Parent_.PartitioningSettings_.MinActivePartitions_ = value;
        return *this;
    }

    TSelf MaxActivePartitions(uint64_t value) {
        Parent_.PartitioningSettings_.MaxActivePartitions_ = value;
        return *this;
    }

    TAutoscalingSettingsBuilder BeginConfigureAutoscalingSettings() {
        return {*this, Parent_.PartitioningSettings_.AutoscalingSettings_};
    }

    TCreateTopicSettings& EndConfigurePartitioningSettings() {
        return Parent_;
    }

private:
    TCreateTopicSettings& Parent_;
};

struct TAlterTopicSettings : public TOperationRequestSettings<TAlterTopicSettings> {

    using TSelf = TAlterTopicSettings;
    using TAlterAttributes = std::map<std::string, std::string>;

    FLUENT_SETTING_OPTIONAL(TDuration, SetRetentionPeriod);

    FLUENT_SETTING_OPTIONAL_VECTOR(ECodec, SetSupportedCodecs);

    FLUENT_SETTING_OPTIONAL(uint64_t, SetRetentionStorageMb);

    FLUENT_SETTING_OPTIONAL(uint64_t, SetPartitionWriteSpeedBytesPerSecond);
    FLUENT_SETTING_OPTIONAL(uint64_t, SetPartitionWriteBurstBytes);

    FLUENT_SETTING_OPTIONAL(EMeteringMode, SetMeteringMode);

    FLUENT_SETTING_VECTOR(TConsumerSettings<TAlterTopicSettings>, AddConsumers);
    FLUENT_SETTING_VECTOR(std::string, DropConsumers);
    FLUENT_SETTING_VECTOR(TAlterConsumerSettings, AlterConsumers);

    FLUENT_SETTING(TAlterAttributes, AlterAttributes);

    TAlterTopicAttributesBuilder BeginAlterAttributes() {
        return TAlterTopicAttributesBuilder(*this);
    }

    TAlterTopicSettings& SetSupportedCodecs(std::vector<ECodec>&& codecs) {
        SetSupportedCodecs_ = std::move(codecs);
        return *this;
    }

    TAlterTopicSettings& SetSupportedCodecs(const std::vector<ECodec>& codecs) {
        SetSupportedCodecs_ = codecs;
        return *this;
    }

    TConsumerSettings<TAlterTopicSettings>& BeginAddConsumer() {
        AddConsumers_.push_back({*this});
        return AddConsumers_.back();
    }

    TConsumerSettings<TAlterTopicSettings>& BeginAddConsumer(const std::string& name) {
        AddConsumers_.push_back({*this, name});
        return AddConsumers_.back();
    }

    TAlterConsumerSettings& BeginAlterConsumer() {
        AlterConsumers_.push_back({*this});
        return AlterConsumers_.back();
    }

    TAlterConsumerSettings& BeginAlterConsumer(const std::string& name) {
        AlterConsumers_.push_back({*this, name});
        return AlterConsumers_.back();
    }

    TAlterPartitioningSettings& BeginAlterPartitioningSettings() {
        AlterPartitioningSettings_.emplace(*this);
        return *AlterPartitioningSettings_;
    }

    TAlterTopicSettings& AlterPartitioningSettings(uint64_t minActivePartitions, uint64_t maxActivePartitions) {
        AlterPartitioningSettings_.emplace(*this);
        AlterPartitioningSettings_->MinActivePartitions(minActivePartitions);
        AlterPartitioningSettings_->MaxActivePartitions(maxActivePartitions);
        return *this;
    }

    std::optional<TAlterPartitioningSettings> AlterPartitioningSettings_;
};

inline TPartitioningSettingsBuilder TCreateTopicSettings::BeginConfigurePartitioningSettings() {
    return {*this};
}

// Settings for drop resource request.
struct TDropTopicSettings : public TOperationRequestSettings<TDropTopicSettings> {
    using TOperationRequestSettings<TDropTopicSettings>::TOperationRequestSettings;
};

// Settings for describe topic request.
struct TDescribeTopicSettings : public TOperationRequestSettings<TDescribeTopicSettings> {
    using TSelf = TDescribeTopicSettings;

    FLUENT_SETTING_DEFAULT(bool, IncludeStats, false);

    FLUENT_SETTING_DEFAULT(bool, IncludeLocation, false);
};

// Settings for describe consumer request.
struct TDescribeConsumerSettings : public TOperationRequestSettings<TDescribeConsumerSettings> {
    using TSelf = TDescribeConsumerSettings;

    FLUENT_SETTING_DEFAULT(bool, IncludeStats, false);

    FLUENT_SETTING_DEFAULT(bool, IncludeLocation, false);
};

// Settings for describe partition request.
struct TDescribePartitionSettings: public TOperationRequestSettings<TDescribePartitionSettings> {
    using TSelf = TDescribePartitionSettings;

    FLUENT_SETTING_DEFAULT(bool, IncludeStats, false);

    FLUENT_SETTING_DEFAULT(bool, IncludeLocation, false);
};

// Settings for commit offset request.
struct TCommitOffsetSettings : public TOperationRequestSettings<TCommitOffsetSettings> {};

}  // namespace NYdb::NTopic
