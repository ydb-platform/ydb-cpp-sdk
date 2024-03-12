#pragma once

#include <client/ydb_driver/driver.h>
#include <client/ydb_table/table.h>

#include <library/cpp/getopt/last_getopt.h>

#include <string>
#include <util/generic/yexception.h>
#include <util/stream/output.h>
#include <util/string/builder.h>
#include <util/string/printf.h>

////////////////////////////////////////////////////////////////////////////////

#define TABLE_SERIES "series"
#define TABLE_SERIES_REV_VIEWS "series_rev_views"

struct TSeries {
    ui64 SeriesId;
    std::string Title;
    std::string SeriesInfo;
    TInstant ReleaseDate;
    ui64 Views;
};

////////////////////////////////////////////////////////////////////////////////

enum class ECmd {
    NONE,
    CREATE_TABLES,
    DROP_TABLES,
    UPDATE_VIEWS,
    LIST_SERIES,
    GENERATE_SERIES,
    DELETE_SERIES,
};

std::string GetCmdList();
ECmd ParseCmd(const char* cmd);

////////////////////////////////////////////////////////////////////////////////

std::string JoinPath(const std::string& prefix, const std::string& path);

////////////////////////////////////////////////////////////////////////////////

class TYdbErrorException : public yexception {
public:
    TYdbErrorException(NYdb::TStatus status)
        : Status(std::move(status))
    { }

    friend IOutputStream& operator<<(IOutputStream& out, const TYdbErrorException& e) {
        out << "Status: " << e.Status.GetStatus();
        if (e.Status.GetIssues()) {
            out << Endl;
            e.Status.GetIssues().PrintTo(out);
        }
        return out;
    }

private:
    NYdb::TStatus Status;
};

inline void ThrowOnError(NYdb::TStatus status) {
    if (!status.IsSuccess()) {
        throw TYdbErrorException(status) << status;
    }
}

////////////////////////////////////////////////////////////////////////////////

int RunCreateTables(NYdb::TDriver& driver, const std::string& prefix, int argc, char** argv);
int RunDropTables(NYdb::TDriver& driver, const std::string& prefix, int argc, char** argv);
int RunUpdateViews(NYdb::TDriver& driver, const std::string& prefix, int argc, char** argv);
int RunListSeries(NYdb::TDriver& driver, const std::string& prefix, int argc, char** argv);
int RunGenerateSeries(NYdb::TDriver& driver, const std::string& prefix, int argc, char** argv);
int RunDeleteSeries(NYdb::TDriver& driver, const std::string& prefix, int argc, char** argv);
