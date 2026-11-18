#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    try {
        auto connection = QSqlDatabase::addDatabase("QODBC");
        connection.setDatabaseName(argc > 1 ? QString::fromUtf8(argv[1])
                                          : qEnvironmentVariable("ODBC_CONNECTION_STRING"));
        if (!connection.open()) throw std::runtime_error(connection.lastError().text().toStdString());

        QSqlQuery query(connection);
        if (!query.prepare("SELECT CAST(? AS Int32) AS answer, CAST(? AS Utf8) AS message")) {
            throw std::runtime_error(query.lastError().text().toStdString());
        }
        query.addBindValue(42);
        query.addBindValue(QStringLiteral("Hello YDB"));
        if (!query.exec()) throw std::runtime_error(query.lastError().text().toStdString());
        while (query.next()) {
            std::cout << query.value(0).toInt() << '\t'
                      << query.value(1).toString().toStdString() << '\n';
        }
        if (query.lastError().isValid()) throw std::runtime_error(query.lastError().text().toStdString());
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
