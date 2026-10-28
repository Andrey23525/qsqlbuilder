#include "Updater.h"
#include "Query.h"

#include <QSqlQuery>

struct Updater::UpdaterPrivate {
    UpdaterPrivate(const Query* q, const QVariantMap& updateValues) : _query(q), _updateValues(updateValues)
    {
    }

    const Query* _query;
    const QVariantMap _updateValues;

    QString _where;
};

const QString Updater::UPDATE_SQL { "UPDATE %1 SET %2 WHERE %3" };

/***************************************************************************************/

Updater::Updater(const Query* q, const QVariantMap& updateValues) : impl(new UpdaterPrivate(q, updateValues))
{
}

Updater::~Updater()
{
}

Updater::Updater(Updater&&) = default;

Updater Updater::where(ClauseUtils&& clause) &&
{
    impl->_where = std::move(clause).sql();
    return std::move(*this);
}

QString Updater::sql() const
{
    QStringList setPart;
    for(const QString& key : impl->_updateValues.keys()) {
        setPart << QString("\"%1\"=%2").arg(key).arg(ClauseUtils::escapeValue(impl->_updateValues[key]));
    }

    return Updater::UPDATE_SQL.arg(impl->_query->tableName()).arg(setPart.join(',')).arg(impl->_where.isEmpty() ? "True" : impl->_where);
}
