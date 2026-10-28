#pragma once

class Query;
class InserterPerformer;

/*!
 * \brief The Inserter class
 * is a INSERT SQL query generator. Two classes are
 * used, so that performing a query without providing values is not possible.
 */
class Inserter {
    Q_DISABLE_COPY(Inserter)
public:
    /*!
     * \brief Inserter  -- constructor, obviously
     * \param q         -- ptr to the Query class, that created it
     * \param fields    -- list of column names in INSERT INTO tbl (...)
     */
    Inserter(const Query* q, const QStringList& fields);
    ~Inserter();

    Inserter(Inserter&&);
    Inserter& operator=(Inserter&&) = default;

    /*!
     * \brief values    -- adds list of values to be inserted to the generator
     * \param data      -- list of inserted values, shoul match by count the number of fields
     * \return          -- returns similar generator, but with the ability to execute the query
     */
    InserterPerformer values(const QVariantList& data) &&;

private:
    struct InserterPrivate;
    std::unique_ptr<InserterPrivate> impl;

    friend class InserterPerformer;
    static const QString INSERT_SQL;
};

/*******************************************************************************************/

/*!
 * \brief The InserterPerformer class
 * is another internal class, similar to Inserter, but
 * alredy propagated with some values, so Database can execute the query.
 * Supports adding more values, like in VALUES(...),(...),(...)
 * IMPORTANT: the functional relies on "... RETURNING id;" SQL syntax,
 * has not been tested without it. That is exactly where existence of
 * primary key (or some of it's replacement) is crusial (provided in Query class
 * or determined by Database class).
 */
class InserterPerformer {
    Q_DISABLE_COPY(InserterPerformer)
public:
    ~InserterPerformer();

    /*!
     * \brief InserterPerformer -- move-construncts the class from Inserter
     * \param inserter          -- Inserter class to be constructed from
     */
    InserterPerformer(Inserter&& inserter);

    InserterPerformer(InserterPerformer&&) = default;
    InserterPerformer& operator=(InserterPerformer&&) = default;

    /*!
     * \brief values    -- adds list of values to be inserted to the generator
     * \param data      -- list of inserted values, shoul match by count the number of fields
     * \return          -- returns this generator, so that more values can be added
     */
    InserterPerformer values(const QVariantList& data) &&;

    /*!
     * \brief query -- the Query class, that created the generator
     * \return      -- ptr to the Query class
     */
    const Query* query() const;

    /*!
     * \brief sql           -- generates the query, Database::execute() performs it
     * \param primaryKey    -- column name in "... RETURNING %1;"
     * \return              -- string with the generated SQL
     */
    QString sql(const QString& primaryKey) const;

private:
    std::unique_ptr<Inserter::InserterPrivate> impl;
};
