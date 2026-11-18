#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlIndex>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QUuid>
#include <QVariant>

#include <iostream>

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    const QString settings = argc > 1 ? QString::fromUtf8(argv[1])
                                     : qEnvironmentVariable("ODBC_CONNECTION_STRING");
    auto connection = QSqlDatabase::addDatabase("QODBC");
    connection.setDatabaseName(settings);
    const QString table = "example_qt_" + QUuid::createUuid().toString(QUuid::Id128);
    bool created = false;
    const auto check = [](bool okay, const auto& source) { if (!okay) throw source.lastError(); };
    const auto query = [&](const QString& sql) {
        QSqlQuery result(connection);
        check(result.exec(sql), result);
        return result;
    };
    const auto scalar = [&](const QString& sql) {
        auto result = query(sql);
        check(result.next(), result);
        return result.value(0);
    };
    try {
        check(connection.open(), connection);
        query("CREATE TABLE " + table + " (id Int64 NOT NULL, name Utf8, note Utf8, amount Double, "
              "payload String, created Date32, seen Timestamp64, active Bool, cost Decimal(22,9), PRIMARY KEY (id))");
        created = true;
        QSqlQuery insert(connection);
        check(insert.prepare("INSERT INTO " + table + " (id, name, note, amount, payload, created, seen, active, cost) "
                             "VALUES (?, ?, ?, ?, ?, ?, ?, ?, CAST(? AS Decimal(22,9)))"), insert);
        insert.addBindValue(qlonglong(4294967297));
        insert.addBindValue(QString::fromUtf8("Привет, 世界 🌍"));
        insert.addBindValue(QVariant(QMetaType::fromType<QString>()));
        insert.addBindValue(12.5);
        insert.addBindValue(QByteArray::fromHex("0041ff"));
        insert.addBindValue(QDate(1960, 2, 29));
        insert.addBindValue(QDateTime(QDate(1960, 2, 29), QTime(12, 34, 56, 123), Qt::UTC));
        insert.addBindValue(true);
        insert.addBindValue(QStringLiteral("1234567890123.123456789"));
        check(insert.exec(), insert);
        std::cout << "inserted: " << insert.numRowsAffected() << '\n';
        insert.bindValue(0, qlonglong(2));
        insert.bindValue(1, QStringLiteral("Second"));
        insert.bindValue(2, QString(6000, 'x'));
        check(insert.exec(), insert);
        auto rows = query("SELECT id, name, note, amount, payload, created, seen, active, CAST(cost AS Utf8) AS cost FROM "
                          + table + " ORDER BY id DESC");
        QStringList names;
        for (int i = 0; i < rows.record().count(); ++i) names << rows.record().fieldName(i);
        std::cout << "columns: " << names.join(", ").toStdString() << '\n';
        while (rows.next()) {
            std::cout << rows.value(0).toLongLong() << ": " << rows.value(1).toString().toStdString()
                      << ", note=" << (rows.isNull(2) ? "NULL" : (std::to_string(rows.value(2).toString().size()) + ":"
                          + QCryptographicHash::hash(rows.value(2).toString().toUtf8(), QCryptographicHash::Sha256).toHex().toStdString()))
                      << ", amount=" << rows.value(3).toDouble()
                      << ", payload=" << rows.value(4).toByteArray().toHex().constData()
                      << ", created=" << rows.value(5).toDate().toString(Qt::ISODate).toStdString()
                      << ", seen=" << rows.value(6).toDateTime().toString(Qt::ISODateWithMs).toStdString()
                      << ", active=" << rows.value(7).toBool() << ", cost=" << rows.value(8).toString().toStdString() << '\n';
        }
        check(!rows.lastError().isValid(), rows);
        std::cout << "primary key: " << connection.primaryIndex(table).fieldName(0).toStdString() << '\n';
        std::cout << "table found: " << connection.tables().contains(table) << '\n';
        std::cout << "updated: " << query("UPDATE " + table + " SET amount = 20 WHERE id = 2").numRowsAffected() << '\n';
        std::cout << "updated missing: " << query("UPDATE " + table + " SET amount = 0 WHERE id = 99").numRowsAffected() << '\n';
        check(connection.transaction(), connection);
        query("UPDATE " + table + " SET amount = 99 WHERE id = 2");
        check(connection.rollback(), connection);
        std::cout << "after rollback: " << scalar("SELECT amount FROM " + table + " WHERE id = 2").toDouble() << '\n';

        auto other = QSqlDatabase::addDatabase("QODBC", "writer");
        other.setDatabaseName(settings);
        check(other.open(), other);
        int attempt = 0;
        for (;;) {
            ++attempt;
            check(connection.transaction(), connection);
            try {
                const double amount = scalar("SELECT amount FROM " + table + " WHERE id = 2").toDouble();
                if (attempt == 1) {
                    // A second client invalidates our read, exercising the retry path.
                    QSqlQuery writer(other);
                    check(writer.exec("UPDATE " + table + " SET amount = amount + 1 WHERE id = 2"), writer);
                }
                query("UPDATE " + table + " SET amount = " + QString::number(amount + 1) + " WHERE id = 2");
                check(connection.commit(), connection);
                break;
            } catch (const QSqlError& error) {
                check(connection.rollback(), connection);
                // QODBC exposes the native YDB ABORTED code rather than SQLSTATE 40001.
                if (error.nativeErrorCode() != "400040" || attempt == 3) throw;
            }
        }
        other.close();
        std::cout << "transaction attempts: " << attempt << '\n';
        std::cout << "after commit: " << scalar("SELECT amount FROM " + table + " WHERE id = 2").toDouble() << '\n';
        auto batch = query("SELECT 10 AS value; SELECT 20 AS value");
        do {
            check(batch.next(), batch);
            std::cout << "result: " << batch.value(0).toInt() << '\n';
        } while (batch.nextResult());
        check(!batch.lastError().isValid(), batch);
        QSqlQuery invalid(connection);
        std::cout << "invalid SQL accepted: " << invalid.exec("SELECT FROM") << '\n';
        std::cout << "connection reused: " << scalar("SELECT 42").toInt() << '\n';
        std::cout << "deleted: " << query("DELETE FROM " + table + " WHERE id = 2").numRowsAffected() << '\n';
        std::cout << "deleted missing: " << query("DELETE FROM " + table + " WHERE id = 99").numRowsAffected() << '\n';
        std::cout << "remaining: " << scalar("SELECT COUNT(*) FROM " + table).toLongLong() << '\n';
        query("DROP TABLE " + table);
    } catch (const QSqlError& error) {
        connection.rollback();
        if (created) QSqlQuery(connection).exec("DROP TABLE " + table);
        std::cerr << error.text().toStdString() << '\n';
        return 1;
    }
}
