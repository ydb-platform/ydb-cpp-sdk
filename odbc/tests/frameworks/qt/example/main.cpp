#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <iostream>
#include <string_view>
int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    if (argc == 2 && std::string_view(argv[1]) == "--qt-version") {
        std::cout << qVersion() << std::endl;
        return 0;
    }
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <dsn-or-connection-string>" << std::endl;
        return 2;
    }
    if (!QSqlDatabase::isDriverAvailable("QODBC")) {
        std::cerr << "Qt QODBC plugin is not available" << std::endl;
        return 3;
    }
    const QString connectionName = QStringLiteral("ydb-odbc-integration");
    {
        QSqlDatabase database = QSqlDatabase::addDatabase("QODBC", connectionName);
        database.setDatabaseName(QString::fromLocal8Bit(argv[1]));
        if (!database.open()) {
            std::cerr << "QODBC connection failed: " << database.lastError().text().toStdString() << std::endl;
            return 4;
        }
        QSqlQuery query(database);
        query.setForwardOnly(true);
        if (!query.exec(QStringLiteral("SELECT 42 AS value"))) {
            std::cerr << "QODBC query failed: " << query.lastError().text().toStdString() << std::endl;
            return 5;
        }
        if (!query.next()) {
            std::cerr << "QODBC returned no row: " << query.lastError().text().toStdString() << std::endl;
            return 6;
        }
        const QVariant value = query.value(0);
        if (value.toInt() != 42) {
            std::cerr << "QODBC returned an unexpected result: type=" << value.typeName() << ", value="
                      << value.toString().toStdString() << std::endl;
            return 7;
        }
        database.close();
    }
    QSqlDatabase::removeDatabase(connectionName);
    return 0;
}
