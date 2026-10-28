#pragma once

/*!
 * Constants of the library, included in pch.h.
 * SQL templates of the generated queries, their %N parts are filled by the generators.
 */

//! SELECT: fields, table, JOIN parts, WHERE clause, tail (GROUP BY, HAVING, ORDER BY, LIMIT, OFFSET)
inline constexpr QLatin1StringView selectSqlTemplate { "SELECT %1 FROM %2 %3 WHERE %4 %5;" };

//! INSERT: table, fields, values, returned column
inline constexpr QLatin1StringView insertSqlTemplate { "INSERT INTO %1 %2 VALUES %3 RETURNING %4;" };

//! UPDATE: table, SET part, WHERE clause
inline constexpr QLatin1StringView updateSqlTemplate { "UPDATE %1 SET %2 WHERE %3;" };

//! DELETE: table, WHERE clause
inline constexpr QLatin1StringView deleteSqlTemplate { "DELETE FROM %1 WHERE %2;" };

//! WHERE clause, that matches all the rows, when no clause was given
inline constexpr QLatin1StringView matchAllSql { "True" };

//! SQL NULL literal
inline constexpr QLatin1StringView nullSql { "NULL" };
