#include "database.h"
#include "deleter.h"
#include "inserter.h"
#include "query.h"
#include "selector.h"
#include "updater.h"

bool Database::_logQueries { false };

/**********************************************************************************/

Database::Database(const QString& driver, const QString& hostname, const QString& dbname, const QString& username, const QString& password)
{
    const QString connectionName = QUuid::createUuid().toString();

    _db = QSqlDatabase::addDatabase(driver, connectionName);
    _db.setHostName(hostname);
    _db.setDatabaseName(dbname);
    _db.setUserName(username);
    _db.setPassword(password);
}

Database::~Database()
{
    const QString connectionName = _db.connectionName();
    _db.close();

    // removeDatabase() expects no QSqlDatabase instances of the connection to be left
    _db = QSqlDatabase();
    QSqlDatabase::removeDatabase(connectionName);
}

void Database::setQueryLoggingEnabled(bool enabled)
{
    Database::_logQueries = enabled;
}

std::expected<void, QSqlError> Database::open()
{
    if(!_db.open()) {
        const QSqlError openError = _db.lastError();
        return std::unexpected(openError);
    }

    return {};
}

std::expected<QVariantList, QSqlError> Database::execute(Selector&& selector)
{
    selector.resolveColumns(*this);

    QString where = selector.where();
    if(where.isEmpty()) {
        where = matchAllSql;
    }

    const QString fields = selector.fields().join(", ");
    const QString tableName = selector.query()->tableName();
    const QString joinTail = selector.joinTail();
    const QString tail = selector.tail();
    const QString sql = selectSqlTemplate.arg(fields, tableName, joinTail, where, tail);

    QVariantList rows;
    const auto executed = performSQL(sql, [&](QSqlQuery& queue) {
        const QSqlRecord record = queue.record();

        rows.reserve(queue.size());
        while(queue.next()) {
            QVariantMap row;
            for(int i = 0; i < record.count(); ++i) {
                const QString fieldName = record.fieldName(i);
                row[fieldName] = queue.value(i);
            }

            rows.append(row);
        }
    });

    if(!executed) {
        return std::unexpected(executed.error());
    }

    return rows;
}

std::expected<QList<int>, QSqlError> Database::execute(InserterPerformer&& inserter)
{
    const Query* query = inserter.query();

    QString primaryKey = query->primaryKeyName();
    if(primaryKey.isEmpty()) {
        primaryKey = primaryKeyName(query->tableName());
    }

    const QString sql = inserter.sql(primaryKey);

    QList<int> ids;
    const auto executed = performSQL(sql, [&](QSqlQuery& q) {
        while(q.next()) {
            ids.append(q.value(0).toInt());
        }
    });

    if(!executed) {
        return std::unexpected(executed.error());
    }

    return ids;
}

std::expected<int, QSqlError> Database::execute(Deleter&& deleter)
{
    const QString sql = deleter.sql();
    return performModifying(sql);
}

std::expected<int, QSqlError> Database::execute(Updater&& updater)
{
    const QString sql = updater.sql();
    return performModifying(sql);
}

std::expected<int, QSqlError> Database::performModifying(const QString& sql)
{
    int affectedRows = 0;
    const auto executed = performSQL(sql, [&](QSqlQuery& q) {
        affectedRows = q.numRowsAffected();
    });

    if(!executed) {
        return std::unexpected(executed.error());
    }

    return affectedRows;
}

std::expected<QSqlQuery, QSqlError> Database::performSQL(const QString& sql, const std::function<void(QSqlQuery&)>& readResult)
{
    QSqlQuery sqlQuery(_db);
    std::expected<void, QSqlError> executed;

    const auto transaction = transact([&] {
        executed = execQuery(sqlQuery, sql, readResult);
    });

    if(!executed) {
        return std::unexpected(executed.error());
    }

    // Inside an outer transact() the query joins it, the outer one reports the errors of the other queries
    if(!transaction && !_inTransaction) {
        return std::unexpected(transaction.error());
    }

    return sqlQuery;
}

std::expected<void, QSqlError> Database::execQuery(QSqlQuery& sqlQuery, const QString& sql,
    const std::function<void(QSqlQuery&)>& readResult)
{
    sqlQuery.exec(sql);

    if(Database::_logQueries) {
        qDebug() << sqlQuery.lastQuery();
    }

    const QSqlError error = sqlQuery.lastError();
    if(error.isValid()) {
        if(!_transactionError.isValid()) {
            _transactionError = error;
        }

        return std::unexpected(error);
    }

    if(readResult) {
        readResult(sqlQuery);
        sqlQuery.finish();
    }

    return {};
}

std::expected<void, QSqlError> Database::transact(std::function<void()>&& operations)
{
    if(_inTransaction) {
        std::move(operations)();

        if(_transactionError.isValid()) {
            return std::unexpected(_transactionError);
        }

        return {};
    }

    if(!_db.transaction()) {
        const QSqlError beginError = transactionError("Transaction was not started");
        return std::unexpected(beginError);
    }

    _inTransaction = true;
    _transactionError = QSqlError();

    std::move(operations)();

    _inTransaction = false;

    if(_transactionError.isValid()) {
        _db.rollback();
        return std::unexpected(_transactionError);
    }

    if(!_db.commit()) {
        const QSqlError commitError = transactionError("Transaction was not commited");

        // some drivers keep the transaction open after a failed commit
        _db.rollback();
        return std::unexpected(commitError);
    }

    return {};
}

QSqlError Database::transactionError(const QString& text) const
{
    const QSqlError driverError = _db.lastError();
    if(driverError.isValid()) {
        return driverError;
    }

    // the driver leaves lastError() empty, if it does not support transactions
    const QString databaseText;
    return QSqlError(text, databaseText, QSqlError::TransactionError);
}

QString Database::primaryKeyName(const QString& tableName) const
{
    const QSqlIndex primaryIndex = _db.primaryIndex(tableName);
    return primaryIndex.fieldName(0);
}

QStringList Database::columnNames(const QString& tableName) const
{
    QStringList result;

    const QSqlRecord columns = _db.record(tableName);
    for(int i = 0; i < columns.count(); ++i) {
        result << columns.fieldName(i);
    }

    return result;
}
