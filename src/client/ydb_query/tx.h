#pragma once

#include <src/api/grpc/ydb_query_v1.grpc.pb.h>

#include <src/client/ydb_types/fluent_settings_helpers.h>

#include <util/stream/output.h>

#include <optional>

namespace NYdb::NQuery {

struct TTxOnlineSettings {
    using TSelf = TTxOnlineSettings;

    FLUENT_SETTING_DEFAULT(bool, AllowInconsistentReads, false);

    TTxOnlineSettings() {}
};

struct TTxSettings {
    using TSelf = TTxSettings;

    TTxSettings()
        : Mode_(TS_SERIALIZABLE_RW) {}

    static TTxSettings SerializableRW() {
        return TTxSettings(TS_SERIALIZABLE_RW);
    }

    static TTxSettings OnlineRO(const TTxOnlineSettings& settings = TTxOnlineSettings()) {
        return TTxSettings(TS_ONLINE_RO).OnlineSettings(settings);
    }

    static TTxSettings StaleRO() {
        return TTxSettings(TS_STALE_RO);
    }

    static TTxSettings SnapshotRO() {
        return TTxSettings(TS_SNAPSHOT_RO);
    }

    void Out(IOutputStream& out) const {
        switch (Mode_) {
        case TS_SERIALIZABLE_RW:
            out << "SerializableRW";
            break;
        case TS_ONLINE_RO:
            out << "OnlineRO";
            break;
        case TS_STALE_RO:
            out << "StaleRO";
            break;
        case TS_SNAPSHOT_RO:
            out << "SnapshotRO";
            break;
        default:
            out << "Unknown";
            break;
        }
    }

    enum ETransactionMode {
        TS_SERIALIZABLE_RW,
        TS_ONLINE_RO,
        TS_STALE_RO,
        TS_SNAPSHOT_RO
    };

    const ETransactionMode Mode_;
    FLUENT_SETTING(TTxOnlineSettings, OnlineSettings);

private:
    TTxSettings(ETransactionMode mode)
        : Mode_(mode) {}
};

struct TTxControl {
    using TSelf = TTxControl;

    static TTxControl Tx(const std::string& txId) {
        return TTxControl(txId);
    }

    static TTxControl BeginTx(const TTxSettings& settings = TTxSettings()) {
        return TTxControl(settings);
    }

    static TTxControl NoTx() {
        return TTxControl();
    }

    const std::optional<std::string> TxId_;
    const std::optional<TTxSettings> TxSettings_;
    FLUENT_SETTING_FLAG(CommitTx);

    bool HasTx() const { return TxId_.has_value() || TxSettings_.has_value(); }

private:
    TTxControl() {}

    TTxControl(const std::string& txId)
        : TxId_(txId) {}

    TTxControl(const TTxSettings& txSettings)
        : TxSettings_(txSettings) {}
};

} // namespace NYdb::NQuery
