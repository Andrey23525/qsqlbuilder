#include "qsqlbuilder/where.h"

ClauseUtils::ClauseUtils(const QString& field, const QString& op, const QString& value)
{
    const QString sql = QString("\"%1\" %2 %3").arg(field, op, value);
    _sql = sql;
}

ClauseUtils ClauseUtils::comparison(const QString& fieldName, const QString& op, const QVariant& value)
{
    const QString text = value.toString();
    const QString escaped = ClauseUtils::escapeValue(text);
    return ClauseUtils(fieldName, op, escaped);
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
    if(value.isNull()) {
        return nullSql;
    }

    QString result;

    switch(value.typeId()) {
        case QMetaType::QDate: {
            const QDate date = value.toDate();
            if(!date.isValid()) {
                return nullSql;
            }

            result = date.toString(Qt::ISODate);
            break;
        }

        case QMetaType::QTime: {
            const QTime time = value.toTime();
            if(!time.isValid()) {
                return nullSql;
            }

            result = time.toString(Qt::ISODate);
            break;
        }

        case QMetaType::QDateTime: {
            const QDateTime dateTime = value.toDateTime();
            if(!dateTime.isValid()) {
                return nullSql;
            }

            result = dateTime.toString(Qt::ISODate);
            break;
        }

        case QMetaType::QString:
        case QMetaType::QChar:
            result = value.toString().trimmed().replace('\'', "''");
            break;

        case QMetaType::Bool:
            result = QString::number(value.toBool());
            break;

        case QMetaType::QByteArray: {
            const QByteArray hex = value.toByteArray().toHex();
            result = QString::fromLatin1(hex);
            break;
        }

        default:
            result = value.toString();
    }

    return QString("'%1'").arg(result);
}

ClauseUtils EQ(const QString& fieldName, const QVariant& value)
{
    return ClauseUtils::comparison(fieldName, "=", value);
}

ClauseUtils NEQ(const QString& fieldName, const QVariant& value)
{
    return ClauseUtils::comparison(fieldName, "!=", value);
}

ClauseUtils LT(const QString& fieldName, const QVariant& value)
{
    return ClauseUtils::comparison(fieldName, "<", value);
}

ClauseUtils GT(const QString& fieldName, const QVariant& value)
{
    return ClauseUtils::comparison(fieldName, ">", value);
}

ClauseUtils LE(const QString& fieldName, const QVariant& value)
{
    return ClauseUtils::comparison(fieldName, "<=", value);
}

ClauseUtils GE(const QString& fieldName, const QVariant& value)
{
    return ClauseUtils::comparison(fieldName, ">=", value);
}

ClauseUtils IN(const QString& fieldName, const QVariantList& values)
{
    QStringList stringValues;
    for(const QVariant& value : values) {
        stringValues << ClauseUtils::escapeValue(value);
    }

    const QString joined = stringValues.join(',');
    const QString list = QString("(%1)").arg(joined);
    return ClauseUtils(fieldName, "IN", list);
}

ClauseUtils IS_NULL(const QString& fieldName)
{
    const QString nullValue = nullSql;
    return ClauseUtils(fieldName, "IS", nullValue);
}
