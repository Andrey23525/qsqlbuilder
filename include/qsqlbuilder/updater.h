#pragma once

class ClauseUtils;
class Query;

/*!
 * \brief The Updater class
 * is an UPDATE query generator. Works as all the
 * other generators, supports clauses.
 * IMPORTANT: if you forget to call "where()" and specify some correct update
 * values, you'll get ALL OF YOU TABLE updated. That's how SQL works, but
 * here it is a bit less obvious way to shoot your foot badly.
 */
class Updater {
    Q_DISABLE_COPY(Updater)
public:
    /*!
     * \brief Updater       -- constructor of the generator, don't use it manually
     * \param q             -- ptr to the Query class, that created it
     * \param updateValues  -- map of values like in "...SET column_name = 'value',...", maps
     * (obviously) column names on values being set
     */
    Updater(const Query* q, const QVariantMap& updateValues);

    /*!
     * \brief ~Updater  -- destructor
     */
    ~Updater();

    /*!
     * \brief Updater   -- move constructor
     * \param rhs       -- generator to be moved from
     */
    Updater(Updater&& rhs) noexcept;

    /*!
     * \brief operator= -- move assignment
     * \param rhs       -- generator to be moved from
     * \return          -- this generator
     */
    Updater& operator=(Updater&& rhs) noexcept;

    /*!
     * \brief where     -- "WHERE ..." clause (see where.h for details)
     * \param clause    -- some aggregated clause
     * \return          -- this generator as rvalue to be reused
     */
    Updater where(ClauseUtils&& clause) &&;

    /*!
     * \brief sql   -- generates the query, Database::execute() performs it
     * \return      -- string with the generated SQL
     */
    QString sql() const;

private:
    struct UpdaterPrivate;
    std::unique_ptr<UpdaterPrivate> impl;
};
