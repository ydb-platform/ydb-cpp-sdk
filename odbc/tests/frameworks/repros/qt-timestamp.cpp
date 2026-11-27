#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QDateTime>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlField>
#include <sql.h>
#include <sqlext.h>
int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    const bool ydb = argc > 1;
    const QString connection = ydb ? QString::fromLocal8Bit(argv[1])
        : QString("Driver={SQLite3};Database=/tmp/qt-timestamp-control.db");
    auto db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName(connection);
    if (!db.open()) { qCritical() << db.lastError(); return 1; }
    QSqlQuery q(db);
    q.exec("DROP TABLE IF EXISTS timestamp_control");
    if (!q.exec(ydb ? "CREATE TABLE timestamp_control (id Int32, v Timestamp64, PRIMARY KEY (id))"
                   : "CREATE TABLE timestamp_control (id INTEGER PRIMARY KEY, v TIMESTAMP)")) {
        qCritical() << q.lastError(); return 2;
    }
    const QDateTime expected(QDate(2150, 1, 5), QTime(14, 0, 0, 123), Qt::OffsetFromUTC, 3600);
    q.prepare("INSERT INTO timestamp_control (id, v) VALUES (?, ?)");
    q.addBindValue(1); q.addBindValue(expected);
    if (!q.exec()) { qCritical() << q.lastError(); return 3; }
    if (!q.exec("SELECT v FROM timestamp_control") || !q.next()) { qCritical() << q.lastError(); return 4; }
    const QDateTime actual = q.value(0).toDateTime();
    qInfo() << "expected" << expected.toString(Qt::ISODateWithMs) << expected.toMSecsSinceEpoch();
    qInfo() << "actual" << actual.toString(Qt::ISODateWithMs) << actual.toMSecsSinceEpoch()
            << "delta ms" << expected.msecsTo(actual) << "equal" << (expected == actual);
    SQLHENV env = SQL_NULL_HENV; SQLHDBC dbc = SQL_NULL_HDBC; SQLHSTMT stmt = SQL_NULL_HSTMT;
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0);
    SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
    auto cs = connection.toUtf8();
    if (!SQL_SUCCEEDED(SQLDriverConnectA(dbc, nullptr, reinterpret_cast<SQLCHAR *>(cs.data()), SQL_NTS,
            nullptr, 0, nullptr, SQL_DRIVER_NOPROMPT))) return 5;
    SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    SQLExecDirectA(stmt, reinterpret_cast<SQLCHAR *>(const_cast<char *>("SELECT v FROM timestamp_control")), SQL_NTS);
    SQLFetch(stmt); SQL_TIMESTAMP_STRUCT value = {}; SQLLEN length = 0;
    auto status = SQLGetData(stmt, 1, SQL_C_TYPE_TIMESTAMP, &value, sizeof(value), &length);
    qInfo() << "raw timestamp" << status << value.year << value.month << value.day
            << value.hour << value.minute << value.second << value.fraction;
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
    q.exec("DROP TABLE timestamp_control");
}
