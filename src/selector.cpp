#include "Selector.h"
#include "Query.h"
#include "database.h"

// Fills in all the columns of the target table if none were given and resolves disambiduation in column names
void Selector::resolveColumns(const Database& db)
{
    const auto& columnNames = db.columnNames(_query->tableName());
    if(_fields.isEmpty())
        _fields = columnNames;

    auto thisColumnSet = QSet<QString>(columnNames.begin(), columnNames.end());
    for(const auto& part : _joinParts) {
        const auto& tableColumnNames = db.columnNames(part._joinTable);
        auto otherColumnSet = QSet<QString>(tableColumnNames.begin(), tableColumnNames.end());
        otherColumnSet.intersect(thisColumnSet);

        for(std::size_t i = 0; i < _fields.count(); ++i) {
            auto fieldName = _fields[i];
            if(otherColumnSet.contains(fieldName)) {
                auto resolutionFieldName =
                    QString("%1.%2").arg(!part._joinDisambigToOther ? _query->tableName() : part._joinTable).arg(fieldName);
                // hacky a bit, but IT WORKS
                _where.replace(fieldName, resolutionFieldName);
                _where.replace('.', "\".\"");
                _fields.replace(i, resolutionFieldName);
            }
        }
    }
}

QString Selector::joinTail() const
{
    if(_joinParts.empty()) {
        return {};
    }

    QString result;

    QStringList joiner;
    joiner.reserve(_joinParts.size());

    for(const auto& part : _joinParts) {
        joiner << part._sql;
    }

    result = joiner.join(' ');

    return result;
}

/***************************************************************************************/

const QString Selector::SELECT_SQL { "SELECT %1 FROM %2 %3 WHERE %4 %5;" };

Selector::Selector(const Query* q, const QStringList& fields) : _query(q), _fields(fields)
{
}

Selector::~Selector()
{
}

Selector::Selector(Selector&&) = default;

Selector Selector::join(const QString& otherTable, const std::pair<QString, QString>& joinColumns, Join::JoinType joinType,
    bool resolveDisambigToOther)
{
    JoinPart part;

    part._joinTable = otherTable;
    part._joinDisambigToOther = resolveDisambigToOther;
    part._sql =
        QString("%1 JOIN %2 on %3")
            .arg(QVariant::fromValue(joinType).toString())
            .arg(otherTable)
            .arg(QString("\"%1\".\"%2\"=\"%3\".\"%4\"").arg(_query->tableName(), joinColumns.first, otherTable, joinColumns.second));

    _joinParts.append(part);
    return std::move(*this);
}

Selector Selector::where(ClauseUtils&& clause) &&
{
    _where = std::move(clause).sql();
    return std::move(*this);
}

Selector Selector::limit(int count) &&
{
    _limit = count > 0 ? QString("LIMIT %1").arg(count) : "";
    return std::move(*this);
}

Selector Selector::orderBy(const QString& field, Order::OrderType selectOrder) &&
{
    _order = QString("ORDER BY %1 %2").arg(field).arg(QVariant::fromValue(selectOrder).toString());
    return std::move(*this);
}

Selector Selector::groupBy(const QString& field) &&
{
    _groupBy = QString("GROUP BY %1").arg(field);
    return std::move(*this);
}

Selector Selector::having(const QString& havingClause) &&
{
    _having = QString("HAVING %1").arg(havingClause);
    return std::move(*this);
}

Selector Selector::offset(int offset) &&
{
    _offset = QString("OFFSET %1").arg(offset);
    return std::move(*this);
}
