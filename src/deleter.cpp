#include "Deleter.h"
#include "Query.h"

#include <QSqlError>
#include <QSqlQuery>

struct Deleter::DeleterPrivate {
    DeleterPrivate(const Query* query, ClauseUtils&& whereClause) : _query(query), _where { std::move(whereClause).sql() }
    {
    }

    const Query* _query;
    QString _where;
};

const QString Deleter::DELETE_SQL { "DELETE FROM %1 WHERE %2;" };

/***************************************************************************************/

Deleter::Deleter(const Query* query, ClauseUtils&& whereClause) : impl(new DeleterPrivate(query, std::forward<ClauseUtils>(whereClause)))
{
}

Deleter::~Deleter()
{
}

Deleter::Deleter(Deleter&&) = default;

QString Deleter::sql() const
{
    return Deleter::DELETE_SQL.arg(impl->_query->tableName()).arg(impl->_where.isEmpty() ? "True" : impl->_where);
}
