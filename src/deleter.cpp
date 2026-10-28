#include "qsqlbuilder/deleter.h"
#include "qsqlbuilder/query.h"
#include "qsqlbuilder/where.h"

struct Deleter::DeleterPrivate {
    DeleterPrivate(const Query* query, const QString& where) : _query(query), _where(where)
    {
    }

    const Query* _query;
    QString _where;
};

/***************************************************************************************/

Deleter::Deleter(const Query* query, ClauseUtils&& whereClause)
{
    const QString where = std::move(whereClause).sql();
    impl = std::make_unique<DeleterPrivate>(query, where);
}

Deleter::~Deleter()
{
}

Deleter::Deleter(Deleter&& rhs) noexcept = default;

Deleter& Deleter::operator=(Deleter&& rhs) noexcept = default;

QString Deleter::sql() const
{
    QString where = impl->_where;
    if(where.isEmpty()) {
        where = matchAllSql;
    }

    const QString tableName = impl->_query->tableName();
    return deleteSqlTemplate.arg(tableName, where);
}
