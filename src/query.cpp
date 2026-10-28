#include "query.h"

#include "deleter.h"
#include "inserter.h"
#include "selector.h"
#include "updater.h"
#include "where.h"

Query::Query(const QString& tableName, const QString& primaryKey) : _tableName(tableName), _primaryKey(primaryKey)
{
}

QString Query::tableName() const
{
    return _tableName;
}

QString Query::primaryKeyName() const
{
    return _primaryKey;
}

Selector Query::select(const QStringList& fields) const
{
    return Selector(this, fields);
}

Inserter Query::insert(const QStringList& fields) const
{
    return Inserter(this, fields);
}

Deleter Query::delete_(ClauseUtils&& whereClause) const
{
    return Deleter(this, std::move(whereClause));
}

Updater Query::update(const QVariantMap& updateValues) const
{
    return Updater(this, updateValues);
}
