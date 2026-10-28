#include "inserter.h"
#include "query.h"
#include "where.h"

struct Inserter::InserterPrivate {
    InserterPrivate(const Query* q, const QStringList& fields) : _query(q), _fields(fields)
    {
    }

    const Query* _query;
    const QStringList _fields;

    QList<QVariantList> _data;
};

/***************************************************************************************/

Inserter::Inserter(const Query* q, const QStringList& fields)
{
    impl = std::make_unique<InserterPrivate>(q, fields);
}

Inserter::~Inserter()
{
}

Inserter::Inserter(Inserter&& rhs) noexcept = default;

Inserter& Inserter::operator=(Inserter&& rhs) noexcept = default;

InserterPerformer Inserter::values(const QVariantList& data) &&
{
    InserterPerformer performer(std::move(*this));
    return std::move(performer).values(data);
}

/***************************************************************************************/

InserterPerformer::~InserterPerformer()
{
}

InserterPerformer::InserterPerformer(Inserter&& inserter) : impl(std::move(inserter.impl))
{
}

InserterPerformer::InserterPerformer(InserterPerformer&& rhs) noexcept = default;

InserterPerformer& InserterPerformer::operator=(InserterPerformer&& rhs) noexcept = default;

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
        for(const QVariant& value : dataTuple) {
            vBlock << ClauseUtils::escapeValue(value);
        }

        const QString values = vBlock.join(',');
        valueTail << QString("(%1)").arg(values);
    }

    const QString tableName = impl->_query->tableName();
    const QString fieldList = impl->_fields.join(',');
    const QString fields = QString("(%1)").arg(fieldList);
    const QString values = valueTail.join(',');
    return insertSqlTemplate.arg(tableName, fields, values, primaryKey);
}
