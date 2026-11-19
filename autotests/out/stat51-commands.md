# SQL command generation verification

Standalone MSVC Release configuration and build passed with QSQLBUILDER_BUILD_TESTS=ON and QSQLBUILDER_ENABLE_LTO=OFF.

CTest: qsqlbuilder_command_tests passed (1/1, 0.03 seconds). Seven exact-output cases cover BEGIN TRANSACTION, COMMIT, ROLLBACK, SQL Server LOCK_TIMEOUT and isolation level, PostgreSQL search_path and quoted time zone syntax. The cases verify that SET grammar is not incorrectly quoted as a bound data value. No database connections or transaction side effects are introduced by the generator API.

This suite checks generated text only. Executing generated commands on actual SQL Server and DuckDB connections is covered by the consuming statistics loader's integration tests.

Whitespace check: git diff --check passed.
