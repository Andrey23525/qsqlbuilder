#pragma once

#include "Where.h"

class Query;

/*!
 * \brief The Deleter class
 * is a DELETE SQL query generator. Supports WHERE clauses,
 * should *never* be used as lvalue or constructed manually.
 * Don't clean tables with it, it is possible, but not efficient.
 */
class Deleter {
    Q_DISABLE_COPY(Deleter)
public:
    /*!
     * \brief Deleter       -- constructs the generator
     * \param q             -- ptr to the Query class, that created it
     * \param whereClause   -- "WHERE ..." clause (see OP namespace for details)
     */
    Deleter(const Query* q, ClauseUtils&& whereClause);
    ~Deleter();

    Deleter(Deleter&&);
    Deleter& operator=(Deleter&&) = default;

    /*!
     * \brief sql   -- generates the query, Database::execute() performs it
     * \return      -- string with the generated SQL
     */
    QString sql() const;

private:
    struct DeleterPrivate;
    std::unique_ptr<DeleterPrivate> impl;

    static const QString DELETE_SQL;
};
