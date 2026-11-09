#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <cstdio>

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    if (argc < 2) return 2;
    auto db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName(argv[1]);
    if (!db.open()) {
        qCritical("%s", qPrintable(db.lastError().text()));
        return 1;
    }
    QSqlQuery query(db);
    if (!query.exec(argc > 2 ? argv[2] : "SELECT CAST(NULL AS VARCHAR(20)) AS value") || !query.next()) {
        qCritical("%s", qPrintable(query.lastError().text()));
        return 1;
    }
    const auto value = query.value(0);
    std::printf("value valid=%d null=%d type=%s queryNull=%d\n", value.isValid(), value.isNull(),
                value.typeName() ? value.typeName() : "invalid", query.isNull(0));
}
