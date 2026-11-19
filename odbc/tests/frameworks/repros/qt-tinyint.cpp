#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlField>
#include <QtSql/QSqlError>
#include <QtCore/QVariant>
#include <sql.h>
#include <sqlext.h>
int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    const bool ydb = argc > 1;
    const QString connection = ydb ? QString::fromLocal8Bit(argv[1])
        : QString("Driver={SQLite3};Database=/tmp/qt-tinyint-control.db");
    auto db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName(connection);
    if (!db.open()) { qCritical() << db.lastError(); return 1; }
    QSqlQuery q(db);
    q.exec("DROP TABLE IF EXISTS tinyint_control");
    if (!q.exec(ydb ? "CREATE TABLE tinyint_control (v Int8, PRIMARY KEY (v))" : "CREATE TABLE tinyint_control (v TINYINT)")) return 2;
    if (!q.exec(ydb ? "INSERT INTO tinyint_control (v) VALUES (CAST(-128 AS Int8))" : "INSERT INTO tinyint_control VALUES (-128)")) return 3;
    if (!q.exec("SELECT v FROM tinyint_control") || !q.next()) return 4;
    qInfo() << "QODBC metadata" << q.record().field(0).metaType().name()
            << "value" << q.value(0) << "as signed char" << int(q.value(0).value<signed char>())
            << "as signed wide" << q.value(0).toLongLong();
    SQLHENV env = SQL_NULL_HENV;
    SQLHDBC dbc = SQL_NULL_HDBC;
    SQLHSTMT stmt = SQL_NULL_HSTMT;
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0);
    SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
    auto cs = connection.toUtf8();
    if (!SQL_SUCCEEDED(SQLDriverConnectA(dbc, nullptr, reinterpret_cast<SQLCHAR *>(cs.data()), SQL_NTS,
            nullptr, 0, nullptr, SQL_DRIVER_NOPROMPT))) return 5;
    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    SQLExecDirectA(stmt, reinterpret_cast<SQLCHAR *>(const_cast<char *>("SELECT v FROM tinyint_control")), SQL_NTS);
    SQLSMALLINT type = 0;
    SQLLEN unsignedFlag = -1, length = 0;
    SQLCHAR columnName[128] = {}; SQLSMALLINT nameLength = 0, decimalDigits = 0, nullable = 0; SQLULEN columnSize = 0;
    SQLDescribeColA(stmt, 1, columnName, sizeof(columnName), &nameLength, &type, &columnSize, &decimalDigits, &nullable);
    SQLColAttributeA(stmt, 1, SQL_DESC_UNSIGNED, nullptr, 0, nullptr, &unsignedFlag);
    SQLFetch(stmt);
    SQLINTEGER signedValue = 0; SQLUINTEGER unsignedValue = 0;
    auto signedStatus = SQLGetData(stmt, 1, SQL_C_SLONG, &signedValue, sizeof(signedValue), &length);
    SQLCloseCursor(stmt);
    SQLExecDirectA(stmt, reinterpret_cast<SQLCHAR *>(const_cast<char *>("SELECT v FROM tinyint_control")), SQL_NTS);
    SQLFetch(stmt);
    auto unsignedStatus = SQLGetData(stmt, 1, SQL_C_ULONG, &unsignedValue, sizeof(unsignedValue), &length);
    qInfo() << "raw metadata SQL type" << type << "unsigned" << unsignedFlag
            << "SQL_C_SLONG" << signedStatus << signedValue
            << "SQL_C_ULONG" << unsignedStatus << unsignedValue;
    if (!SQL_SUCCEEDED(unsignedStatus)) {
        SQLCHAR state[6] = {}; SQLCHAR message[1024] = {};
        SQLGetDiagRecA(SQL_HANDLE_STMT, stmt, 1, state, nullptr, message, sizeof(message), nullptr);
        qInfo() << "unsigned read diagnostic" << reinterpret_cast<const char *>(state) << reinterpret_cast<const char *>(message);
    }
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
    q.exec("DROP TABLE tinyint_control");
}
