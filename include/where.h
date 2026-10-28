#pragma once

/*!
 * Here goes a namespase of helpers that implement "WHERE ..." support
 * in all the generators. Can be extended with other conditions, perhaps.
 */
/*!
 * \brief The ClauseUtils class
 * is an internal class, that translates C++ boolean logic into "WHERE ..." statements.
 * It optimizes nothing, no syntax tree parsing here, so don't abuse your queries with too much
 * of ClauseUtilss. Executed ClauseUtils is guaranteed to have the same interpretation when calculated by
 * database as you've written in your sources. Just using braces here =)
 */
class ClauseUtils {
public:
    /*!
     * \brief ClauseUtils    -- is a constructor of smth like " col='val' "
     * \param field     -- column name in the ClauseUtils
     * \param op        -- ClauseUtils operation (=,>,<, IN, etc.)
     * \param value     -- value in the ClauseUtils
     */
    ClauseUtils(const QString& field, const QString& op, const QString& value);

    /*!
     * \brief operator ! -- is a SQL negation, uses "NOT (...)"
     * \return           -- this mini-generator as rvalue to be reused
     */
    ClauseUtils operator!() &&;

    /*!
     * \brief operator ! -- is a SQL logical conjunction, uses "(...) AND (...)"
     * \return           -- this mini-generator as rvalue to be reused
     */
    ClauseUtils operator&&(ClauseUtils&& other) &&;

    /*!
     * \brief operator ! -- is a SQL logical union, uses "(...) OR (...)"
     * \return           -- this mini-generator as rvalue to be reused
     */
    ClauseUtils operator||(ClauseUtils&& other) &&;

    /*!
     * \brief sql    -- returns the curently generated SQL
     * \return          -- string with the accumulated ClauseUtils
     */
    QString sql() &&;

    /*!
     * \brief escapeValue   -- escapes values due to database rules. Honestly taken from
     * the QSqlDriver class (same idea, written in a more simple way). Has not been tested fully,
     * the target was PostgreSQL, so binary data may not be supported by other DB engines. It's not
     * only used by the ClauseUtilss, also internally used in other generators.
     * DB identifiers are just escaped like "id" everywhere through the code.
     * \param value         -- value to be escaped propely
     * \return              -- string represetnation of the value to be used
     */
    static QString escapeValue(const QVariant& value);

private:
    QString _sql;
};

/*!
 * \brief EQ        -- helper, that constructs " col='val' " ClauseUtils part
 * \param fieldName -- column name
 * \param value     -- value to be used
 * \return          -- ClauseUtils entity (see the above class)
 */
ClauseUtils EQ(const QString& fieldName, const QVariant& value);

/*!
 * \brief NEQ       -- helper, that constructs " col!='val' " ClauseUtils part
 * \param fieldName -- column name
 * \param value     -- value to be used
 * \return          -- ClauseUtils entity (see the above class)
 */
ClauseUtils NEQ(const QString& fieldName, const QVariant& value);

/*!
 * \brief LT        -- helper, that constructs " col<'val' " ClauseUtils part
 * \param fieldName -- column name
 * \param value     -- value to be used
 * \return          -- ClauseUtils entity (see the above class)
 */
ClauseUtils LT(const QString& fieldName, const QVariant& value);

/*!
 * \brief GT        -- helper, that constructs " col>'val' " ClauseUtils part
 * \param fieldName -- column name
 * \param value     -- value to be used
 * \return          -- ClauseUtils entity (see the above class)
 */
ClauseUtils GT(const QString& fieldName, const QVariant& value);

/*!
 * \brief LE        -- helper, that constructs " col<='val' " ClauseUtils part
 * \param fieldName -- column name
 * \param value     -- value to be used
 * \return          -- ClauseUtils entity (see the above class)
 */
ClauseUtils LE(const QString& fieldName, const QVariant& value);

/*!
 * \brief GE        -- helper, that constructs " col>='val' " ClauseUtils part
 * \param fieldName -- column name
 * \param value     -- value to be used
 * \return          -- ClauseUtils entity (see the above class)
 */
ClauseUtils GE(const QString& fieldName, const QVariant& value);

/*!
 * \brief IN        -- helper, that constructs "col IN ('val1', 'val2', ...)" ClauseUtils part
 * \param fieldName -- column name
 * \param values    -- values to be used
 * \return          -- ClauseUtils entity (see the above class)
 */
ClauseUtils IN(const QString& fieldName, const QVariantList& values);

/*!
 * \brief EQ        -- helper, that constructs "col IS NULL" ClauseUtils part
 * \param fieldName -- column name
 * \return          -- ClauseUtils entity (see the above class)
 */
ClauseUtils IS_NULL(const QString& fieldName);
