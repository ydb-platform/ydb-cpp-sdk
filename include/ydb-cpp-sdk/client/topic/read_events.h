#pragma once

#include "codecs.h"
#include "events_common.h"

#include <ydb-cpp-sdk/util/datetime/base.h>

namespace NYdb::NTopic {

//! Partition session.
struct TPartitionSession: public TThrRefBase, public TPrintable<TPartitionSession> {
    using TPtr = TIntrusivePtr<TPartitionSession>;

public:
    //! Request partition session status.
    //! Result will come to TPartitionSessionStatusEvent.
    virtual void RequestStatus() = 0;

    //!
    //! Properties.
    //!

    //! Unique identifier of partition session.
    //! It is unique within one read session.
    ui64 GetPartitionSessionId() const {
        return PartitionSessionId;
    }

    //! Topic path.
    const std::string& GetTopicPath() const {
        return TopicPath;
    }

    //! Partition id.
    ui64 GetPartitionId() const {
        return PartitionId;
    }

protected:
    ui64 PartitionSessionId;
    std::string TopicPath;
    ui64 PartitionId;
};

template<>
void TPrintable<TPartitionSession>::DebugString(TStringBuilder& res, bool) const;

//! Events for read session.
struct TReadSessionEvent {
    class TPartitionSessionAccessor {
    public:
        TPartitionSessionAccessor(TPartitionSession::TPtr partitionSession);

        virtual ~TPartitionSessionAccessor() = default;

        virtual const TPartitionSession::TPtr& GetPartitionSession() const;

    protected:
        TPartitionSession::TPtr PartitionSession;
    };

    //! Event with new data.
    //! Contains batch of messages from single partition session.
    struct TDataReceivedEvent : public TPartitionSessionAccessor, public TPrintable<TDataReceivedEvent> {
        struct TMessageInformation {
            TMessageInformation(ui64 offset,
                                std::string producerId,
                                ui64 seqNo,
                                TInstant createTime,
                                TInstant writeTime,
                                TWriteSessionMeta::TPtr meta,
                                TMessageMeta::TPtr messageMeta,
                                ui64 uncompressedSize,
                                std::string messageGroupId);
            ui64 Offset;
            std::string ProducerId;
            ui64 SeqNo;
            TInstant CreateTime;
            TInstant WriteTime;
            TWriteSessionMeta::TPtr Meta;
            TMessageMeta::TPtr MessageMeta;
            ui64 UncompressedSize;
            std::string MessageGroupId;
        };

        class TMessageBase : public TPrintable<TMessageBase> {
        public:
            TMessageBase(const std::string& data, TMessageInformation info);

            virtual ~TMessageBase() = default;

            virtual const std::string& GetData() const;

            virtual void Commit() = 0;

            //! Message offset.
            ui64 GetOffset() const;

            //! Producer id.
            const std::string& GetProducerId() const;

            //! Message group id.
            const std::string& GetMessageGroupId() const;

            //! Sequence number.
            ui64 GetSeqNo() const;

            //! Message creation timestamp.
            TInstant GetCreateTime() const;

            //! Message write timestamp.
            TInstant GetWriteTime() const;

            //! Metainfo.
            const TWriteSessionMeta::TPtr& GetMeta() const;

            //! Message level meta info.
            const TMessageMeta::TPtr& GetMessageMeta() const;

        protected:
            std::string Data;
            TMessageInformation Information;
        };

        //! Single message.
        struct TMessage: public TMessageBase, public TPartitionSessionAccessor, public TPrintable<TMessage> {
            using TPrintable<TMessage>::DebugString;

            TMessage(const std::string& data, std::exception_ptr decompressionException, TMessageInformation information,
                     TPartitionSession::TPtr partitionSession);

            //! User data.
            //! Throws decompressor exception if decompression failed.
            const std::string& GetData() const override;

            //! Commits single message.
            void Commit() override;

            bool HasException() const;

        private:
            std::exception_ptr DecompressionException;
        };

        struct TCompressedMessage: public TMessageBase,
                                   public TPartitionSessionAccessor,
                                   public TPrintable<TCompressedMessage> {
            using TPrintable<TCompressedMessage>::DebugString;

            TCompressedMessage(ECodec codec, const std::string& data, TMessageInformation information,
                               TPartitionSession::TPtr partitionSession);

            virtual ~TCompressedMessage() {
            }

            //! Message codec
            ECodec GetCodec() const;

            //! Uncompressed size.
            ui64 GetUncompressedSize() const;

            //! Commits all offsets in compressed message.
            void Commit() override;

        private:
            ECodec Codec;
        };

    public:
        TDataReceivedEvent(std::vector<TMessage> messages, std::vector<TCompressedMessage> compressedMessages,
                           TPartitionSession::TPtr partitionSession);

        bool HasCompressedMessages() const {
            return !CompressedMessages.empty();
        }

        size_t GetMessagesCount() const {
            return Messages.size() + CompressedMessages.size();
        }

        //! Get messages.
        std::vector<TMessage>& GetMessages() {
            CheckMessagesFilled(false);
            return Messages;
        }

        const std::vector<TMessage>& GetMessages() const {
            CheckMessagesFilled(false);
            return Messages;
        }

        //! Get compressed messages.
        std::vector<TCompressedMessage>& GetCompressedMessages() {
            CheckMessagesFilled(true);
            return CompressedMessages;
        }

        const std::vector<TCompressedMessage>& GetCompressedMessages() const {
            CheckMessagesFilled(true);
            return CompressedMessages;
        }

        //! Commits all messages in batch.
        void Commit();

    private:
        void CheckMessagesFilled(bool compressed) const {
            Y_ABORT_UNLESS(!Messages.empty() || !CompressedMessages.empty());
            if (compressed && CompressedMessages.empty()) {
                ythrow yexception() << "cannot get compressed messages, parameter decompress=true for read session";
            }
            if (!compressed && Messages.empty()) {
                ythrow yexception() << "cannot get decompressed messages, parameter decompress=false for read session";
            }
        }

    private:
        std::vector<TMessage> Messages;
        std::vector<TCompressedMessage> CompressedMessages;
        std::vector<std::pair<ui64, ui64>> OffsetRanges;
    };

    //! Acknowledgement for commit request.
    struct TCommitOffsetAcknowledgementEvent: public TPartitionSessionAccessor,
                                              public TPrintable<TCommitOffsetAcknowledgementEvent> {
        TCommitOffsetAcknowledgementEvent(TPartitionSession::TPtr partitionSession, ui64 committedOffset);

        //! Committed offset.
        //! This means that from now the first available
        //! message offset in current partition
        //! for current consumer is this offset.
        //! All messages before are committed and futher never be available.
        ui64 GetCommittedOffset() const {
            return CommittedOffset;
        }

    private:
        ui64 CommittedOffset;
    };

    //! Server command for creating and starting partition session.
    struct TStartPartitionSessionEvent: public TPartitionSessionAccessor,
                                        public TPrintable<TStartPartitionSessionEvent> {
        explicit TStartPartitionSessionEvent(TPartitionSession::TPtr, ui64 committedOffset, ui64 endOffset);

        //! Current committed offset in partition session.
        ui64 GetCommittedOffset() const {
            return CommittedOffset;
        }

        //! Offset of first not existing message in partition session.
        ui64 GetEndOffset() const {
            return EndOffset;
        }

        //! Confirm partition session creation.
        //! This signals that user is ready to receive data from this partition session.
        //! If maybe is empty then no rewinding
        void Confirm(std::optional<ui64> readOffset = std::nullopt, std::optional<ui64> commitOffset = std::nullopt);

    private:
        ui64 CommittedOffset;
        ui64 EndOffset;
    };

    //! Server command for stopping and destroying partition session.
    //! Server can destroy partition session gracefully
    //! for rebalancing among all topic clients.
    struct TStopPartitionSessionEvent: public TPartitionSessionAccessor, public TPrintable<TStopPartitionSessionEvent> {
        TStopPartitionSessionEvent(TPartitionSession::TPtr partitionSession, ui64 committedOffset);

        //! Last offset of the partition session that was committed.
        ui64 GetCommittedOffset() const {
            return CommittedOffset;
        }

        //! Confirm partition session destruction.
        //! Confirm has no effect if TPartitionSessionClosedEvent for same partition session with is received.
        void Confirm();

    private:
        ui64 CommittedOffset;
    };

    //! Server command for ending partition session.
    //! This is a hint that all messages from the partition have been read and will no longer appear, and that the client must commit offsets.
    struct TEndPartitionSessionEvent: public TPartitionSessionAccessor, public TPrintable<TEndPartitionSessionEvent> {
        TEndPartitionSessionEvent(TPartitionSession::TPtr partitionSession, std::vector<ui32>&& adjacentPartitionIds, std::vector<ui32>&& childPartitionIds);

        //! A list of the partition IDs that also participated in the partition's merge.
        const std::vector<ui32> GetAdjacentPartitionIds() const {
            return AdjacentPartitionIds;
        }

        //! A list of partition IDs that were obtained as a result of merging or splitting this partition.
        const std::vector<ui32> GetChildPartitionIds() const {
            return ChildPartitionIds;
        }

        //! Confirm partition session destruction.
        //! Confirm has no effect if TPartitionSessionClosedEvent for same partition session with is received.
        void Confirm();

    private:
        std::vector<ui32> AdjacentPartitionIds;
        std::vector<ui32> ChildPartitionIds;
    };

    //! Status for partition session requested via TPartitionSession::RequestStatus()
    struct TPartitionSessionStatusEvent: public TPartitionSessionAccessor,
                                         public TPrintable<TPartitionSessionStatusEvent> {
        TPartitionSessionStatusEvent(TPartitionSession::TPtr partitionSession, ui64 committedOffset, ui64 readOffset,
                                     ui64 endOffset, TInstant writeTimeHighWatermark);

        //! Committed offset.
        ui64 GetCommittedOffset() const {
            return CommittedOffset;
        }

        //! Offset of next message (that is not yet read by session).
        ui64 GetReadOffset() const {
            return ReadOffset;
        }

        //! Offset of first not existing message in partition.
        ui64 GetEndOffset() const {
            return EndOffset;
        }

        //! Write time high watermark.
        //! Write timestamp of next message written to this partition will be no less than this.
        TInstant GetWriteTimeHighWatermark() const {
            return WriteTimeHighWatermark;
        }

    private:
        ui64 CommittedOffset = 0;
        ui64 ReadOffset = 0;
        ui64 EndOffset = 0;
        TInstant WriteTimeHighWatermark;
    };

    //! Event that signals user about
    //! partition session death.
    //! This could be after graceful stop of partition session
    //! or when connection with partition was lost.
    struct TPartitionSessionClosedEvent: public TPartitionSessionAccessor,
                                         public TPrintable<TPartitionSessionClosedEvent> {
        enum class EReason {
            StopConfirmedByUser,
            Lost,
            ConnectionLost,
        };

    public:
        TPartitionSessionClosedEvent(TPartitionSession::TPtr partitionSession, EReason reason);

        EReason GetReason() const {
            return Reason;
        }

    private:
        EReason Reason;
    };

    using TEvent = std::variant<TDataReceivedEvent,
                                TCommitOffsetAcknowledgementEvent,
                                TStartPartitionSessionEvent,
                                TStopPartitionSessionEvent,
                                TEndPartitionSessionEvent,
                                TPartitionSessionStatusEvent,
                                TPartitionSessionClosedEvent,
                                TSessionClosedEvent>;
};

//! Set of offsets to commit.
//! Class that could store offsets in order to commit them later.
//! This class is not thread safe.
class TDeferredCommit {
public:
    //! Add message to set.
    void Add(const TReadSessionEvent::TDataReceivedEvent::TMessage& message);

    //! Add all messages from dataReceivedEvent to set.
    void Add(const TReadSessionEvent::TDataReceivedEvent& dataReceivedEvent);

    //! Add offsets range to set.
    void Add(const TPartitionSession::TPtr& partitionSession, ui64 startOffset, ui64 endOffset);

    //! Add offset to set.
    void Add(const TPartitionSession::TPtr& partitionSession, ui64 offset);

    //! Commit all added offsets.
    void Commit();

    TDeferredCommit();
    TDeferredCommit(const TDeferredCommit&) = delete;
    TDeferredCommit(TDeferredCommit&&);
    TDeferredCommit& operator=(const TDeferredCommit&) = delete;
    TDeferredCommit& operator=(TDeferredCommit&&);

    ~TDeferredCommit();

private:
    class TImpl;
    std::unique_ptr<TImpl> Impl;
};

//! Events debug strings.
template<>
void TPrintable<TReadSessionEvent::TDataReceivedEvent::TMessageBase>::DebugString(TStringBuilder& ret, bool printData) const;
template<>
void TPrintable<TReadSessionEvent::TDataReceivedEvent::TMessage>::DebugString(TStringBuilder& ret, bool printData) const;
template<>
void TPrintable<TReadSessionEvent::TDataReceivedEvent::TCompressedMessage>::DebugString(TStringBuilder& ret, bool printData) const;
template<>
void TPrintable<TReadSessionEvent::TDataReceivedEvent>::DebugString(TStringBuilder& ret, bool printData) const;
template<>
void TPrintable<TReadSessionEvent::TCommitOffsetAcknowledgementEvent>::DebugString(TStringBuilder& ret, bool printData) const;
template<>
void TPrintable<TReadSessionEvent::TStartPartitionSessionEvent>::DebugString(TStringBuilder& ret, bool printData) const;
template<>
void TPrintable<TReadSessionEvent::TStopPartitionSessionEvent>::DebugString(TStringBuilder& ret, bool printData) const;
template<>
void TPrintable<TReadSessionEvent::TPartitionSessionStatusEvent>::DebugString(TStringBuilder& ret, bool printData) const;
template<>
void TPrintable<TReadSessionEvent::TPartitionSessionClosedEvent>::DebugString(TStringBuilder& ret, bool printData) const;
template<>
void TPrintable<TSessionClosedEvent>::DebugString(TStringBuilder& ret, bool printData) const;

std::string DebugString(const TReadSessionEvent::TEvent& event);

}
