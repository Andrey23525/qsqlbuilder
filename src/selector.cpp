#include "selector.h"
#include "database.h"
#include "query.h"
#include "where.h"

void Selector::resolveColumns(const Database& db)
{
    const QString tableName = _query->tableName();
    const QStringList columnNames = db.columnNames(tableName);
    if(_fields.isEmpty()) {
        _fields = columnNames;
    }

    for(const auto& part : _joinParts) {
        const QStringList otherColumnNames = db.columnNames(part._joinTable);
        const QString resolutionTable = part._joinDisambigToOther ? part._joinTable : tableName;

        for(qsizetype i = 0; i < _fields.count(); ++i) {
            const QString fieldName = _fields[i];
            const bool inThisTable = columnNames.contains(fieldName);
            const bool inOtherTable = otherColumnNames.contains(fieldName);
            if(!inThisTable || !inOtherTable) {
                continue;
            }

            const QString resolutionFieldName = QString("%1.%2").arg(resolutionTable, fieldName);
            // hacky a bit, but IT WORKS
            _where.replace(fieldName, resolutionFieldName);
            _where.replace('.', "\".\"");
            _fields.replace(i, resolutionFieldName);
        }
    }
}

QString Selector::joinTail() const
{
    if(_joinParts.empty()) {
        return {};
    }

    QStringList joiner;
    joiner.reserve(_joinParts.size());

    for(const auto& part : _joinParts) {
        joiner << part._sql;
    }

    return joiner.join(' ');
}

QString Selector::tail() const
{
    const QStringList parts { _groupBy, _having, _order, _limit, _offset };
    return parts.join(' ');
}

/***************************************************************************************/

Selector::Selector(const Query* q, const QStringList& fields) : _query(q), _fields(fields)
{
}

Selector::~Selector()
{
}

Selector::Selector(Selector&& rhs) noexcept = default;

Selector Selector::join(const QString& otherTable, const std::pair<QString, QString>& joinColumns, Join::JoinType joinType,
    bool resolveDisambigToOther)
{
    const QString tableName = _query->tableName();
    const QString& thisColumn = joinColumns.first;
    const QString& otherColumn = joinColumns.second;
    const QString condition = QString("\"%1\".\"%2\"=\"%3\".\"%4\"").arg(tableName, thisColumn, otherTable, otherColumn);

    const QString joinTypeName = QVariant::fromValue(joinType).toString();
    const QString sql = QString("%1 JOIN %2 on %3").arg(joinTypeName, otherTable, condition);

    const JoinPart part { sql, otherTable, resolveDisambigToOther };
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
    if(count <= 0) {
        _limit.clear();
        return std::move(*this);
    }

    _limit = QString("LIMIT %1").arg(count);
    return std::move(*this);
}

Selector Selector::orderBy(const QString& field, Order::OrderType selectOrder) &&
{
    const QString orderName = QVariant::fromValue(selectOrder).toString();
    _order = QString("ORDER BY %1 %2").arg(field, orderName);
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
