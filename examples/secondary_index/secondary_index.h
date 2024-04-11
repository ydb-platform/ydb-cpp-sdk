#pragma once

#include <src/client/ydb_driver/driver.h>
#include <src/client/ydb_table/table.h>

#include <library/cpp/getopt/last_getopt.h>

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

    friend std::ostream& operator<<(std::ostream& out, const TYdbErrorException& e) {
        out << "Status: " << ToString(e.Status.GetStatus());
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
