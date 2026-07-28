// Database setup adapter for SOCI's unchanged common ODBC test suite.

#include "soci/odbc/soci-odbc.h"
#include "soci/soci.h"
#include "test-context.h"

#include <string>

using namespace soci;
using namespace soci::tests;

std::string connectString;
backend_factory const& backEnd = *soci::factory_odbc();

namespace {

struct table_creator_one final : table_creator_base
{
    explicit table_creator_one(session& sql)
        : table_creator_base(sql)
    {
        sql << "create table soci_test("
               "id Int32, val Int32, c Utf8, str Utf8, sh Int16, ll Int64, "
               "ul Uint64, d Double, num76 Decimal(7, 6), tm Timestamp, "
               "i1 Int32, i2 Int32, i3 Int32, name Utf8, primary key(id))";
    }
};

struct table_creator_two final : table_creator_base
{
    explicit table_creator_two(session& sql)
        : table_creator_base(sql)
    {
        sql << "create table soci_test("
               "num_float Double, num_int Int32, name Utf8, "
               "sometime Timestamp, chr Utf8, primary key(name))";
    }
};

struct table_creator_three final : table_creator_base
{
    explicit table_creator_three(session& sql)
        : table_creator_base(sql)
    {
        sql << "create table soci_test("
               "name Utf8 not null, phone Utf8, primary key(name))";
    }
};

struct clob_table_creator final : table_creator_base
{
    explicit clob_table_creator(session& sql)
        : table_creator_base(sql)
    {
        sql << "create table soci_test("
               "id Int32, s Text, primary key(id))";
    }
};

class test_context final : public test_context_common
{
public:
    std::string get_backend_name() const override
    {
        return "odbc";
    }

    std::string to_date_time(std::string const& value) const override
    {
        return "{ts '" + value + "'}";
    }

    table_creator_base* table_creator_1(session& sql) const override
    {
        return new table_creator_one(sql);
    }

    table_creator_base* table_creator_2(session& sql) const override
    {
        return new table_creator_two(sql);
    }

    table_creator_base* table_creator_3(session& sql) const override
    {
        return new table_creator_three(sql);
    }

    table_creator_base* table_creator_4(session&) const override
    {
        return nullptr;
    }

    table_creator_base* table_creator_clob(session& sql) const override
    {
        return new clob_table_creator(sql);
    }

    bool has_transactions_support(session&) const override
    {
        return false;
    }

    std::string sql_length(std::string const& value) const override
    {
        return "length(" + value + ")";
    }
};

test_context context;

} // namespace
