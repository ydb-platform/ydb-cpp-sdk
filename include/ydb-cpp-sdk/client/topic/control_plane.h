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
    
enum class EMeteringMode : ui32 {
    Unspecified = 0,
    ReservedCapacity = 1,
    RequestUnits = 2,

    Unknown = std::numeric_limits<int>::max(),
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

    ui64 GetStoreSizeBytes() const;
    TDuration GetMaxWriteTimeLag() const;
    TInstant GetMinLastWriteTime() const;
    ui64 GetBytesWrittenPerMinute() const;
    ui64 GetBytesWrittenPerHour() const;
    ui64 GetBytesWrittenPerDay() const;

private:
    ui64 StoreSizeBytes_;
    TInstant MinLastWriteTime_;
    TDuration MaxWriteTimeLag_;
    ui64 BytesWrittenPerMinute_;
    ui64 BytesWrittenPerHour_;
    ui64 BytesWrittenPerDay_;
};


class TPartitionStats {
public:
    TPartitionStats(const Ydb::Topic::PartitionStats& partitionStats);

    ui64 GetStartOffset() const;
    ui64 GetEndOffset() const;
    ui64 GetStoreSizeBytes() const;
    TDuration GetMaxWriteTimeLag() const;
    TInstant GetLastWriteTime() const;
    ui64 GetBytesWrittenPerMinute() const;
    ui64 GetBytesWrittenPerHour() const;
    ui64 GetBytesWrittenPerDay() const;

private:
    ui64 StartOffset_;
    ui64 EndOffset_;
    ui64 StoreSizeBytes_;
    TInstant LastWriteTime_;
    TDuration MaxWriteTimeLag_;
    ui64 BytesWrittenPerMinute_;
    ui64 BytesWrittenPerHour_;
    ui64 BytesWrittenPerDay_;

};

class TPartitionConsumerStats {
public:
    TPartitionConsumerStats(const Ydb::Topic::DescribeConsumerResult::PartitionConsumerStats& partitionStats);
    ui64 GetCommittedOffset() const;
    ui64 GetLastReadOffset() const;
    std::string GetReaderName() const;
    std::string GetReadSessionId() const;

private:
    ui64 CommittedOffset_;
    i64 LastReadOffset_;
    std::string ReaderName_;
    std::string ReadSessionId_;
};

// Topic partition location
class TPartitionLocation {
public:
    TPartitionLocation(const Ydb::Topic::PartitionLocation& partitionLocation);
    i32 GetNodeId() const;
    i64 GetGeneration() const;

private:
    // Node identificator.
    i32 NodeId_ = 1;

    // Partition generation.
    i64 Generation_ = 2;
};

class TPartitionInfo {
public:
    TPartitionInfo(const Ydb::Topic::DescribeTopicResult::PartitionInfo& partitionInfo);
    TPartitionInfo(const Ydb::Topic::DescribeConsumerResult::PartitionInfo& partitionInfo);

    ui64 GetPartitionId() const;
    bool GetActive() const;
    const std::vector<ui64> GetChildPartitionIds() const;
    const std::vector<ui64> GetParentPartitionIds() const;

    const std::optional<TPartitionStats>& GetPartitionStats() const;
    const std::optional<TPartitionConsumerStats>& GetPartitionConsumerStats() const;
    const std::optional<TPartitionLocation>& GetPartitionLocation() const;

private:
    ui64 PartitionId_;
    bool Active_;
    std::vector<ui64> ChildPartitionIds_;
    std::vector<ui64> ParentPartitionIds_;
    std::optional<TPartitionStats> PartitionStats_;
    std::optional<TPartitionConsumerStats> PartitionConsumerStats_;
    std::optional<TPartitionLocation> PartitionLocation_;
};

class TPartitioningSettings {
public:
    TPartitioningSettings() : MinActivePartitions_(0), PartitionCountLimit_(0){}
    TPartitioningSettings(const Ydb::Topic::PartitioningSettings& settings);
    TPartitioningSettings(ui64 minActivePartitions, ui64 partitionCountLimit)
        : MinActivePartitions_(minActivePartitions)
        , PartitionCountLimit_(partitionCountLimit) {
    }

    ui64 GetMinActivePartitions() const;
    ui64 GetPartitionCountLimit() const;
private:
    ui64 MinActivePartitions_;
    ui64 PartitionCountLimit_;
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

    ui32 GetTotalPartitionsCount() const;

    const std::vector<TPartitionInfo>& GetPartitions() const;

    const std::vector<ECodec>& GetSupportedCodecs() const;

    const TDuration& GetRetentionPeriod() const;

    std::optional<ui64> GetRetentionStorageMb() const;

    ui64 GetPartitionWriteSpeedBytesPerSecond() const;

    ui64 GetPartitionWriteBurstBytes() const;

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
    std::optional<ui64> RetentionStorageMb_;
    ui64 PartitionWriteSpeedBytesPerSecond_;
    ui64 PartitionWriteBurstBytes_;
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


struct TCreateTopicSettings : public TOperationRequestSettings<TCreateTopicSettings> {

    using TSelf = TCreateTopicSettings;
    using TAttributes = std::map<std::string, std::string>;

    FLUENT_SETTING(TPartitioningSettings, PartitioningSettings);

    FLUENT_SETTING_DEFAULT(TDuration, RetentionPeriod, TDuration::Hours(24));

    FLUENT_SETTING_VECTOR(ECodec, SupportedCodecs);

    FLUENT_SETTING_DEFAULT(ui64, RetentionStorageMb, 0);
    FLUENT_SETTING_DEFAULT(EMeteringMode, MeteringMode, EMeteringMode::Unspecified);

    FLUENT_SETTING_DEFAULT(ui64, PartitionWriteSpeedBytesPerSecond, 0);
    FLUENT_SETTING_DEFAULT(ui64, PartitionWriteBurstBytes, 0);

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

    TCreateTopicSettings& PartitioningSettings(ui64 minActivePartitions, ui64 partitionCountLimit) {
        PartitioningSettings_ = TPartitioningSettings(minActivePartitions, partitionCountLimit);
        return *this;
    }
};


struct TAlterTopicSettings : public TOperationRequestSettings<TAlterTopicSettings> {

    using TSelf = TAlterTopicSettings;
    using TAlterAttributes = std::map<std::string, std::string>;

    FLUENT_SETTING_OPTIONAL(TPartitioningSettings, AlterPartitioningSettings);

    FLUENT_SETTING_OPTIONAL(TDuration, SetRetentionPeriod);

    FLUENT_SETTING_OPTIONAL_VECTOR(ECodec, SetSupportedCodecs);

    FLUENT_SETTING_OPTIONAL(ui64, SetRetentionStorageMb);

    FLUENT_SETTING_OPTIONAL(ui64, SetPartitionWriteSpeedBytesPerSecond);
    FLUENT_SETTING_OPTIONAL(ui64, SetPartitionWriteBurstBytes);

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

    TAlterTopicSettings& AlterPartitioningSettings(ui64 minActivePartitions, ui64 partitionCountLimit) {
        AlterPartitioningSettings_ = TPartitioningSettings(minActivePartitions, partitionCountLimit);
        return *this;
    }
};

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

}
