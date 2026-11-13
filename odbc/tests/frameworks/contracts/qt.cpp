#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

#include <atomic>
#include <barrier>
#include <future>
#include <iostream>
#include <stdexcept>

void Check(bool condition, const QString& message) {
    if (!condition) throw std::runtime_error(message.toStdString());
}

class Database {
    static inline std::atomic<unsigned> Next{0};
    QString Name = "contract_" + QString::number(Next.fetch_add(1));
public:
    QSqlDatabase Db = QSqlDatabase::addDatabase("QODBC", Name);
    explicit Database(const QString& connection) {
        Db.setDatabaseName(connection);
        if (!Db.open()) {
            const auto error = Db.lastError().text();
            Db = {}; QSqlDatabase::removeDatabase(Name);
            throw std::runtime_error(error.toStdString());
        }
    }
    ~Database() { Db.close(); Db = {}; QSqlDatabase::removeDatabase(Name); }
    QSqlQuery Query(const QString& sql) {
        QSqlQuery query(Db);
        Check(query.exec(sql), query.lastError().text());
        return query;
    }
    int Exec(const QString& sql) { return Query(sql).numRowsAffected(); }
    QVariant Scalar(const QString& sql) {
        auto query = Query(sql); Check(query.next(), query.lastError().text()); return query.value(0);
    }
};

void Contract(const QString& name, const QString& connection) {
    Database db(connection);
    const QString table = "contract_qt_" + name;
    const bool usesTable = name == "binding" || name == "row_count"
        || name == "transactions" || name == "independent_connections";
    const auto cleanup = [&] {
        if (usesTable) {
            db.Db.rollback();
            db.Exec("DROP TABLE IF EXISTS " + table);
        }
    };
    try {
        if (usesTable) {
            db.Exec("DROP TABLE IF EXISTS " + table);
            db.Exec("CREATE TABLE " + table + " (id Int32, txt Utf8, missing Utf8, PRIMARY KEY (id))");
        }
        if (name == "connection") {
            Check(db.Scalar("SELECT 42 AS answer").toInt() == 42, "Scalar differs");
        } else if (name == "binding") {
            QSqlQuery query(db.Db);
            Check(query.prepare("INSERT INTO " + table + " (id, txt, missing) VALUES (?, ?, ?)"), query.lastError().text());
            query.addBindValue(7); query.addBindValue(QString::fromUtf8("Привет, 世界"));
            query.addBindValue(QVariant(QMetaType::fromType<QString>()));
            Check(query.exec(), query.lastError().text());
            Check(query.exec("SELECT id, txt, missing FROM " + table), query.lastError().text());
            Check(query.next(), "Missing bound row");
            Check(query.value(0).toInt() == 7 && query.value(1).toString() == QString::fromUtf8("Привет, 世界")
                  && query.value(2).isNull(), "Bound values differ");
        } else if (name == "metadata") {
            auto query = db.Query("SELECT 42 AS answer, 'text' AS label");
            Check(query.record().count() == 2 && query.record().fieldName(0) == "answer"
                  && query.record().fieldName(1) == "label", "Column metadata differs");
        } else if (name == "row_count") {
            Check(db.Exec("INSERT INTO " + table + " (id, txt) VALUES (1, 'a'), (2, 'b')") == 2, "Insert count differs");
            Check(db.Exec("UPDATE " + table + " SET txt = 'changed'") == 2, "Update count differs");
            Check(db.Exec("DELETE FROM " + table + " WHERE id = 2") == 1, "Delete count differs");
        } else if (name == "transactions") {
            Check(db.Db.transaction(), db.Db.lastError().text());
            db.Exec("INSERT INTO " + table + " (id) VALUES (1)");
            Check(db.Db.rollback(), db.Db.lastError().text());
            Check(db.Scalar("SELECT COUNT(*) FROM " + table).toInt() == 0, "Rollback changed rows");
            Check(db.Db.transaction(), db.Db.lastError().text());
            db.Exec("INSERT INTO " + table + " (id) VALUES (2)");
            Check(db.Db.commit(), db.Db.lastError().text());
            Check(db.Scalar("SELECT COUNT(*) FROM " + table).toInt() == 1, "Commit lost rows");
        } else if (name == "independent_connections") {
            Database other(connection);
            Check(db.Db.transaction(), db.Db.lastError().text());
            db.Exec("INSERT INTO " + table + " (id) VALUES (1)");
            Check(other.Scalar("SELECT COUNT(*) FROM " + table).toInt() == 0, "Uncommitted data leaked");
            Check(db.Db.commit(), db.Db.lastError().text());
            Check(other.Scalar("SELECT COUNT(*) FROM " + table).toInt() == 1, "Independent connection missed commit");
        } else if (name == "cleanup") {
            { auto unread = db.Query("SELECT 1 UNION ALL SELECT 2"); Check(unread.next(), "Missing row"); }
            Check(db.Scalar("SELECT 9").toInt() == 9, "Unread result prevented reuse");
            Database replacement(connection);
            Check(replacement.Scalar("SELECT 10").toInt() == 10, "New connection failed");
        } else if (name == "multiple_results") {
            auto query = db.Query("SELECT 1 AS first UNION ALL SELECT 2 AS first ORDER BY first; SELECT 'next' AS label, 9 AS number; SELECT 3 AS last");
            Check(query.next() && query.value(0).toInt() == 1, "First result differs");
            Check(query.nextResult() && query.record().count() == 2
                  && query.record().fieldName(0) == "label" && query.record().fieldName(1) == "number", "Second schema differs");
            Check(query.next() && query.value(0).toString() == "next" && query.value(1).toInt() == 9, "Second result differs");
            Check(query.nextResult() && query.record().fieldName(0) == "last", "Third schema differs");
            Check(query.next() && query.value(0).toInt() == 3, "Third result differs");
            Check(!query.nextResult(), "Unexpected extra result");
            Check(query.exec("SELECT 11") && query.next() && query.value(0).toInt() == 11, "Reexecution failed");
        } else if (name == "concurrency") {
            std::barrier start(2);
            const auto work = [&] {
                start.arrive_and_wait(); Database other(connection);
                for (int i = 0; i < 10; ++i) Check(other.Scalar("SELECT 42").toInt() == 42, "Concurrent result differs");
            };
            auto first = std::async(std::launch::async, work);
            auto second = std::async(std::launch::async, work);
            first.get(); second.get();
        }
        cleanup();
    } catch (...) {
        try { cleanup(); } catch (...) {}
        throw;
    }
}

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    if (argc != 2) return 2;
    bool failed = false;
    for (const char* name : {"connection", "binding", "metadata", "row_count", "transactions",
                            "independent_connections", "cleanup", "multiple_results", "concurrency"}) {
        QByteArray error;
        try { Contract(name, QString::fromUtf8(argv[1])); }
        catch (const std::exception& exception) { error = exception.what(); failed = true; }
        std::cout << "@YDB_CONTRACT\t" << name << '\t' << (error.isEmpty() ? "passed" : "failed")
                  << '\t' << error.toHex().constData() << '\n';
    }
    return failed;
}
