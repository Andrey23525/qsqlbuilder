#include "updater.h"
#include "query.h"
#include "where.h"

struct Updater::UpdaterPrivate {
    UpdaterPrivate(const Query* q, const QVariantMap& updateValues) : _query(q), _updateValues(updateValues)
    {
    }

    const Query* _query;
    const QVariantMap _updateValues;

    QString _where;
};

/***************************************************************************************/

Updater::Updater(const Query* q, const QVariantMap& updateValues)
{
    impl = std::make_unique<UpdaterPrivate>(q, updateValues);
}

Updater::~Updater()
{
}

Updater::Updater(Updater&& rhs) noexcept = default;

Updater& Updater::operator=(Updater&& rhs) noexcept = default;

Updater Updater::where(ClauseUtils&& clause) &&
{
    impl->_where = std::move(clause).sql();
    return std::move(*this);
}

QString Updater::sql() const
{
    QStringList setPart;
    for(auto it = impl->_updateValues.cbegin(); it != impl->_updateValues.cend(); ++it) {
        const QString value = ClauseUtils::escapeValue(it.value());
        setPart << QString("\"%1\"=%2").arg(it.key(), value);
    }

    QString where = impl->_where;
    if(where.isEmpty()) {
        where = matchAllSql;
    }

    const QString tableName = impl->_query->tableName();
    const QString setSql = setPart.join(',');
    return updateSqlTemplate.arg(tableName, setSql, where);
}
