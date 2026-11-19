#include "qsqlbuilder/query.h"

/// @brief Checks transaction commands and representative dialect-specific SET expressions.
int main()
{
    const auto cases = QList<std::pair<QString, QString>> {
        { Query::beginTransaction(), QStringLiteral("BEGIN TRANSACTION;") },
        { Query::commit(), QStringLiteral("COMMIT;") },
        { Query::rollback(), QStringLiteral("ROLLBACK;") },
        { Query::set(QStringLiteral("LOCK_TIMEOUT"), QStringLiteral("30000")), QStringLiteral("SET LOCK_TIMEOUT 30000;") },
        { Query::set(QStringLiteral("TRANSACTION ISOLATION LEVEL"), QStringLiteral("SERIALIZABLE")),
            QStringLiteral("SET TRANSACTION ISOLATION LEVEL SERIALIZABLE;") },
        { Query::set(QStringLiteral("search_path TO"), QStringLiteral("public, reporting")),
            QStringLiteral("SET search_path TO public, reporting;") },
        { Query::set(QStringLiteral("TIME ZONE"), QStringLiteral("'Europe/Paris'")),
            QStringLiteral("SET TIME ZONE 'Europe/Paris';") },
    };
    for(const auto& item : cases) {
        if(item.first != item.second) {
            qCritical() << "Unexpected command:" << item.first << "expected:" << item.second;
            return 1;
        }
    }
    qInfo() << cases.size() << "command cases passed";
    return 0;
}
