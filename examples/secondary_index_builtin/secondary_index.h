#pragma once

#include <src/client/ydb_driver/driver.h>
#include <src/client/ydb_table/table.h>

#include <src/library/getopt/last_getopt.h>
#include <string>
#include <util/generic/yexception.h>
#include <util/stream/output.h>
#include <util/string/builder.h>

#define TABLE_USERS "users"
#define TABLE_SERIES "series"

using NYdb::TResultSetParser;

enum class TCommand {
    CREATE,
    INSERT,
    SELECT,
    DROP,
    SELECT_JOIN,
    NONE
};

struct TUser {
    ui64 UserId;
    std::string Name;
    ui32 Age;
    TUser(ui64 userId = 0, std::string name = "", ui32 age = 0)
        : UserId(userId)
        , Name(name)
        , Age(age) {}
};

struct TSeries {
    ui64 SeriesId;
    std::string Title;
    TInstant ReleaseDate;
    std::string Info;
    ui64 Views;
    ui64 UploadedUserId;

    TSeries(ui64 seriesId = 0, std::string title = "", TInstant releaseDate = TInstant::Days(0),
            std::string info = "", ui64 views = 0, ui64 uploadedUserId = 0)
        : SeriesId(seriesId)
        , Title(title)
        , ReleaseDate(releaseDate)
        , Info(info)
        , Views(views)
        , UploadedUserId(uploadedUserId) {}
};

class TYdbErrorException: public yexception {
public:
    TYdbErrorException(NYdb::TStatus status)
        : Status(std::move(status))
    { }

    friend std::ostream& operator<<(std::ostream& out, const TYdbErrorException&  e) {
        out << "Status:" << ToString(e.Status.GetStatus());
        if (e.Status.GetIssues()) {
            out << std::endl;
            e.Status.GetIssues().PrintTo(out);
        }
        return out;
    }
private:
    NYdb::TStatus Status;
};

inline void ThrowOnError(NYdb::TStatus status) {
    if (!status.IsSuccess()){
        throw TYdbErrorException(status) << status;
    }
}

std::string GetCommandsList();
TCommand Parse(const char *stringCmnd);
std::string JoinPath(const std::string& prefix, const std::string& path);

void ParseSelectSeries(std::vector<TSeries>& parseResult, TResultSetParser&& parser);

int Create(NYdb::TDriver& driver, const std::string& path);
int Insert(NYdb::TDriver& driver, const std::string& path);
int Drop(NYdb::TDriver& driver, const std::string& path);
int SelectJoin(NYdb::TDriver& driver, const std::string& path, int argc, char **argv);
int Select(NYdb::TDriver& driver, const std::string& path, int argc, char **argv);
