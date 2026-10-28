#pragma once

class ClauseUtils;
class Selector;
class Inserter;
class Deleter;
class Updater;

/*!
 * \brief The Query class
 * is the prime class, that creates query classes for the chosen table.
 * It does not touch the database: the rvalue-only generators it provides are passed
 * to Database::execute(), that performs them. They are rvalue-only, because reusing them
 * is undefined in terms of common sense, but you can reuse this Query class. Provides all
 * the basics, CRUD. It is supposed that all tables have primary key, not that it won't work
 * without those, but the classes were tesed on the data where they exist, use-case was the similar.
 */
class Query {
public:
    /*!
     * \brief Query      -- constructor
     * \param tableName  -- name of the table to be used in the current set of queries
     * \param primaryKey -- primary key name, in case Qt will not be able to determine it
     */
    Query(const QString& tableName = QString(), const QString& primaryKey = QString());

    /*!
     * \brief select    -- creates SELECT query generator
     * \param fields    -- column names in SELECT ... FROM, all columns of the table if empty
     * \return          -- select generator
     */
    Selector select(const QStringList& fields = QStringList()) const;

    /*!
     * \brief insert    -- creates INSERT query generator
     * \param fields    -- column names in INSERT INTO tbl (...)
     * \return          -- insert query generator
     */
    Inserter insert(const QStringList& fields) const;

    /*!
     * \brief delete_       -- creates DELETE query generator
     * \param whereClause   -- "WHERE ..." clause (see where.h for details)
     * \return              -- delete query generator
     */
    Deleter delete_(ClauseUtils&& whereClause) const;

    /*!
     * \brief update        -- creates UPDATE query generator
     * \param updateValues  -- map of [column : value] to be set
     * \return              -- update query generator
     */
    Updater update(const QVariantMap& updateValues) const;

    /*!
     * \brief tableName -- name of the table, chosen as primary queries' target
     * \return          -- name of the table, of course
     */
    QString tableName() const;

    /*!
     * \brief primaryKeyName -- name of the primary key column, set in the constructor
     * \return               -- primary key column name, empty if Database should determine it
     */
    QString primaryKeyName() const;

private:
    QString _tableName;
    QString _primaryKey;
};
