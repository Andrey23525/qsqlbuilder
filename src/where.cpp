#include "where.h"
#include "Query.h"

ClauseUtils::ClauseUtils(const QString& field, const QString& op, const QString& value)
    : _sql(QString("\"%1\" %2 %3").arg(field, op, value))
{
}

ClauseUtils ClauseUtils::operator!() &&
{
    _sql = QString("NOT (%1)").arg(_sql);
    return std::move(*this);
}

ClauseUtils ClauseUtils::operator&&(ClauseUtils&& other) &&
{
    _sql = QString("(%1) AND (%2)").arg(_sql, other._sql);
    return std::move(*this);
}

ClauseUtils ClauseUtils::operator||(ClauseUtils&& other) &&
{
    _sql = QString("(%1) OR (%2)").arg(_sql, other._sql);
    return std::move(*this);
}

QString ClauseUtils::sql() &&
{
    return std::move(_sql);
}

QString ClauseUtils::escapeValue(const QVariant& value)
{
    /* Honestly taken from QSqlDriver class */
    const auto NULL_STR = QStringLiteral("NULL");

    if(value.isNull())
        return NULL_STR;

    QString result;

    switch(value.type()) {
        case QVariant::Int:
        case QVariant::UInt:
            result = value.toString();
            break;

        case QVariant::Date:
            result = value.toDate().isValid() ? value.toDate().toString(Qt::ISODate) : NULL_STR;
            break;
        case QVariant::Time:
            result = value.toTime().isValid() ? value.toTime().toString(Qt::ISODate) : NULL_STR;
            break;
        case QVariant::DateTime:
            result = value.toDateTime().isValid() ? value.toDateTime().toString(Qt::ISODate) : NULL_STR;
            break;

        case QVariant::String:
        case QVariant::Char:
            result = value.toString().trimmed().replace('\'', "''");
            break;

        case QVariant::Bool:
            result = QString::number(value.toBool());
            break;

        case QVariant::ByteArray: {
            static const char hexchars[] = "0123456789abcdef";
            QString res;
            QByteArray ba = value.toByteArray();
            for(int i = 0; i < ba.size(); ++i) {
                uchar s = static_cast<uchar>(ba[i]);
                res += QLatin1Char(hexchars[s >> 4]);
                res += QLatin1Char(hexchars[s & 0x0f]);
            }
            result = res;
            break;
        }

        default:
            result = value.toString();
    }

    return QString("'%1'").arg(result);
}

ClauseUtils EQ(const QString& fieldName, const QVariant& value)
{
    return ClauseUtils { fieldName, "=", ClauseUtils::escapeValue(value.toString()) };
}

ClauseUtils NEQ(const QString& fieldName, const QVariant& value)
{
    return ClauseUtils { fieldName, "!=", ClauseUtils::escapeValue(value.toString()) };
}

ClauseUtils LT(const QString& fieldName, const QVariant& value)
{
    return ClauseUtils { fieldName, "<", ClauseUtils::escapeValue(value.toString()) };
}

ClauseUtils GT(const QString& fieldName, const QVariant& value)
{
    return ClauseUtils { fieldName, ">", ClauseUtils::escapeValue(value.toString()) };
}

ClauseUtils LE(const QString& fieldName, const QVariant& value)
{
    return ClauseUtils { fieldName, "<=", ClauseUtils::escapeValue(value.toString()) };
}

ClauseUtils GE(const QString& fieldName, const QVariant& value)
{
    return ClauseUtils { fieldName, ">=", ClauseUtils::escapeValue(value.toString()) };
}

ClauseUtils IN(const QString& fieldName, const QVariantList& values)
{
    QStringList stringValues;
    for(const QVariant& value : values)
        stringValues << ClauseUtils::escapeValue(value);

    return ClauseUtils { fieldName, "IN", QString("(%1)").arg(stringValues.join(',')) };
}

ClauseUtils IS_NULL(const QString& fieldName)
{
    return ClauseUtils { fieldName, "IS", ClauseUtils::escapeValue(QVariant()) };
}
