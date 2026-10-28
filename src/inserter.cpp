#include "Inserter.h"
#include "Query.h"
#include "Where.h"

#include <QSqlError>
#include <QSqlQuery>


struct Inserter::InserterPrivate {
    InserterPrivate(const Query* q, const QStringList& fields) : _query(q), _fields(fields)
    {
    }

    const Query* _query;
    const QStringList _fields;

    QList<QVariantList> _data;
};

/***************************************************************************************/

const QString Inserter::INSERT_SQL { "INSERT INTO %1 %2 VALUES %3 RETURNING %4;" };

Inserter::Inserter(const Query* q, const QStringList& fields) : impl(new InserterPrivate(q, fields))
{
}

Inserter::~Inserter()
{
}

Inserter::Inserter(Inserter&&) = default;

InserterPerformer Inserter::values(const QVariantList& data) &&
{
    return InserterPerformer(std::move(*this)).values(data);
}

/***************************************************************************************/

InserterPerformer::~InserterPerformer()
{
}

InserterPerformer::InserterPerformer(Inserter&& inserter) : impl(std::move(inserter.impl))
{
}

InserterPerformer InserterPerformer::values(const QVariantList& data) &&
{
    impl->_data.append(data);
    return std::move(*this);
}

const Query* InserterPerformer::query() const
{
    return impl->_query;
}

QString InserterPerformer::sql(const QString& primaryKey) const
{
    QStringList valueTail;
    for(const QVariantList& dataTuple : impl->_data) {
        QStringList vBlock;
        for(const QVariant& value : dataTuple)
            vBlock << ClauseUtils::escapeValue(value);

        valueTail << QString("(%1)").arg(vBlock.join(','));
    }

    return Inserter::INSERT_SQL.arg(impl->_query->tableName())
        .arg(QString("(%1)").arg(impl->_fields.join(',')))
        .arg(valueTail.join(','))
        .arg(primaryKey);
}
