#include "database.h"
#include "deleter.h"
#include "inserter.h"
#include "query.h"
#include "selector.h"
#include "updater.h"

#include <stdexcept>

// QSqlDatabase leaves lastError() empty, if the driver does not support transactions
static QSqlError transactionError(const QSqlDatabase& db, const QString& text)
{
    return db.lastError().isValid() ? db.lastError() : QSqlError(text, QString(), QSqlError::TransactionError);
}

bool Database::LOG_QUERIES { false };

/**********************************************************************************/

Database::Database(const QString& driver, const QString& hostname, const QString& dbname, const QString& username, const QString& password)
    : _db(QSqlDatabase::addDatabase(driver, QUuid::createUuid().toString()))
{
    _db.setHostName(hostname);
    _db.setDatabaseName(dbname);
    _db.setUserName(username);
    _db.setPassword(password);

    if(!_db.open()) {
        const QString error = _db.lastError().text();
        removeConnection();
        throw std::runtime_error(QString("Database was not opened! =( %1").arg(error).toStdString());
    }
}

Database::~Database()
{
    _db.close();
    removeConnection();
}

void Database::removeConnection()
{
    const QString connectionName = _db.connectionName();

    // removeDatabase() expects no QSqlDatabase instances of the connection to be left
    _db = QSqlDatabase();
    QSqlDatabase::removeDatabase(connectionName);
}

void Database::setQueryLoggingEnabled(bool enabled)
{
    Database::LOG_QUERIES = enabled;
}

QVariantList Database::execute(Selector&& selector)
{
    QVariantList result;

    selector.resolveColumns(*this);

    const auto tail = QStringList({ selector.groupBy(), selector.having(), selector.order(), selector.limit(), selector.offset() });

    const QString sql = Selector::SELECT_SQL.arg(selector.fields().join(", "))
                            .arg(selector.query()->tableName())
                            .arg(selector.joinTail())
                            .arg(selector.where().isEmpty() ? "True" : selector.where())
                            .arg(tail.join(" "));

    performSQL(sql, [&](QSqlQuery& queue) {
        QSqlRecord record = queue.record();

        result.reserve(queue.size());
        while(queue.next()) {
            QVariantMap resultRow;
            for(int i = 0; i < record.count(); ++i) {
                resultRow[record.fieldName(i)] = queue.value(i);
            }

            result.append(resultRow);
        }
    });

    return hasError() ? QVariantList() : result;
}

QList<int> Database::execute(InserterPerformer&& inserter)
{
    QList<int> result;

    const Query* query = inserter.query();
    const QString primaryKey = !query->primaryKeyName().isEmpty() ? query->primaryKeyName() : primaryKeyName(query->tableName());

    performSQL(inserter.sql(primaryKey), [&](QSqlQuery& q) {
        while(q.next())
            result.append(q.value(0).toInt());
    });

    return hasError() ? QList<int>() : result;
}

bool Database::execute(Deleter&& deleter)
{
    int affectedRows = 0;

    performSQL(deleter.sql(), [&](QSqlQuery& q) {
        affectedRows = q.numRowsAffected();
    });

    return !hasError() && affectedRows > 0;
}

bool Database::execute(Updater&& updater)
{
    int affectedRows = 0;

    performSQL(updater.sql(), [&](QSqlQuery& q) {
        affectedRows = q.numRowsAffected();
    });

    return !hasError() && affectedRows > 0;
}

QSqlQuery Database::performSQL(const QString& sql, const std::function<void(QSqlQuery&)>& readResult)
{
    QSqlQuery sqlQuery(_db);

    transact([&] {
        sqlQuery.exec(sql);

        if(Database::LOG_QUERIES)
            qDebug() << sqlQuery.lastQuery();

        _lastError = sqlQuery.lastError();
        if(hasError()) {
            if(!_transactionError.isValid())
                _transactionError = _lastError;
            return;
        }

        if(readResult) {
            readResult(sqlQuery);
            sqlQuery.finish();
        }
    });

    return sqlQuery;
}

bool Database::transact(std::function<void()>&& operations)
{
    if(_inTransaction) {
        std::move(operations)();
        return !_transactionError.isValid();
    }

    if(!_db.transaction()) {
        _lastError = transactionError(_db, "Transaction was not started");
        return false;
    }

    _inTransaction = true;
    _transactionError = QSqlError();

    try {
        std::move(operations)();
    }
    catch(...) {
        _inTransaction = false;
        _db.rollback();
        throw;
    }

    _inTransaction = false;

    if(_transactionError.isValid()) {
        _db.rollback();
        _lastError = _transactionError;
        return false;
    }

    if(!_db.commit()) {
        _lastError = transactionError(_db, "Transaction was not commited");
        // some drivers keep the transaction open after a failed commit
        _db.rollback();
        return false;
    }

    return true;
}

QSqlError Database::lastError() const
{
    return _lastError;
}

bool Database::hasError() const
{
    return _lastError.isValid();
}

QString Database::primaryKeyName(const QString& tableName) const
{
    return _db.primaryIndex(tableName).fieldName(0);
}

QStringList Database::columnNames(const QString& tableName) const
{
    QStringList result;

    QSqlRecord columns = _db.record(tableName);
    for(int i = 0; i < columns.count(); ++i)
        result << columns.fieldName(i);

    return result;
}
