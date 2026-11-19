# QSqlBuilder

A small C++23 library, built over Qt, that provides rapid building & executing of SQL queries, hiding raw query text. 

### What is it? What for?

Well, here is a simple & easy-to-use (I hope!) SQL query builder using Qt's types and database interaction classes.
It uses all the same QSqlDatabase, QSqlQuery, etc., but you'll be saved the effort of writing 100500 strings with SQL (*and checking errors in each of them*) in your sources.
Most of the documentation is provided as Doxygen comments.

For some sophisticated purposed you'd probably like to use some ORM, for example an excellent [sqlpp11](https://github.com/rbock/sqlpp11), but sometimes you *just don't need that much*. That could be a fine time-saver in the case.

When a thing like the descibed here could be of use:

* You are writing a small Qt backend (whatever reason there be, ex.: small application service with API, that was my case)
* Your Qt client is small and doesn't need a real ORM (ex.: SQLITE used)
* You need code mocking or *fast implementing* of smth.
* You are going to change your database structure a lot later and that's where a good C++ ORM will bring you pain 

In other languages there exist reflection, LINQ (in C#) and other conveniences. But in C++ a small custom builder can help instead.
Basic CRUD (create + read + update + delete) are implemented, by "create" an INSERT query is meant.

## Usage

There are two main classes. `Query` builds queries for a table: it never touches the database, so it is cheap, copyable and can be reused as long as you like. `Database` owns the connection and performs the queries, built by `Query`. The generators in between (`Selector`, `Inserter`, `Deleter`, `Updater`) are internal and designed to be rvalue-only: you build a query in one expression and pass it to `Database::execute()`. By the way, despite the methods returning some internal classes all the time, it won't cost you much in terms of performance, the classes are not only lightweight but also heavily use copy elision everywhere.

Every query is performed in its own SQL transaction: it is commited on success and rollbacked on failure. Several queries can be grouped into a single transaction with `transact()`.

The library throws no exceptions. Everything that can fail returns `std::expected`: either the result or the `QSqlError`, received from the database exactly as Qt reports it.

Thread-safety is questionable, it is equal to the thread-safety of QSqlDatabase class. Of course, the generators consist of string manipulations only, that's pretty safe, but if you start a transaction in one thread and perform a SELECT in the other -- that should cause a failure. Exactly as it does with the QSqlDatabase.

 The rest is better shown by example. The functions for WHERE clauses (`EQ`, `LE`, `IN`, `IS_NULL`, ...) live in `where.h`.

### Configuration

```cpp
Database db("QPSQL", "127.0.0.1", "db_name", "root", ""); // specify your connection parameters here

const auto opened = db.open();
if(!opened) {
    qWarning() << opened.error().text();
    return;
}

Database::setQueryLoggingEnabled(true); // enable/disable logging via qDebug()
```
Logging is essentially useful to check the generated SQL for better understanding the concept, it's likely that examples do not cover all the caveats.
`Database` is neither copyable nor movable, it closes and removes its connection on destruction.

### Raw SQL (something too complex to be generated)

```cpp
QStringList names;
const auto q = db.performSQL("SELECT _id, name FROM my_table LIMIT 3;", [&](QSqlQuery& query) {
    while(query.next()) {
        names << query.value("name").toString();
    }
});
```
Should be something *really* complex. The callback reads the result before the transaction is commited: some drivers refuse to commit while a SELECT is still active.
Without the callback you get the `QSqlQuery` itself, as usually, the connection would be already configured.

### Select

```cpp
const Query query("my_table");

const auto res = db.execute(query
                .select({"id", "name"})
                .where(EQ("some_field", "some_value") && LE("id", 1234)) // WHERE ("some_field" = 'some_value') AND ("id" <= 1234)
                .orderBy("id", Order::DESC)
                .limit(3)
                .offset(20));

const auto res = db.execute(query
                .select({"id", "name", "guid"})
                .where(IN("id", {23, 55, 66, 77}) || !IS_NULL("some_key")) // WHERE ("id" IN ('23', '55', '66', '77')) OR NOT("some_key" IS NULL)
                .orderBy("id", Order::ASC));
``` 
You'll get a `std::expected<QVariantList, QSqlError>`, each element of the list is a `QVariantMap` with same column names as specified in `select()`, aliases ("col AS smth") are also supported. Actually, aliases should save you from trying to figure out, how the `join()` is working in details.
Empty `select()` means all the columns of the table.

NOTE: the WHERE part is implemented cpp-style, it generates lots of braces, but that is how your natural cpp logic is being translated into SQL without surprising permutations. Order of the calls does not matter, except for joins. That WHERE clauses are used by all the generators internally.

### Delete

```cpp
const auto deleted = db.execute(query.delete_(IN("id", {11, 22, 33})));
```
Pretty simple, result is the number of deleted rows.
 
### Update

```cpp
const auto updated = db.execute(query
            .update({{"descr", "OLOLOLO"}})
            .where(LE("id", 100500)));
```
Same as DELETE, result is the number of updated rows.

### Insert

```cpp
const auto ids = db.execute(query
            .insert({"some_number", "guid", "date"})
            .values({42, QUuid::createUuid().toString(), QDate::currentDate()})
            .values({42, QUuid::createUuid().toString(), QDate::currentDate().addDays(3)})
            .values({42, QUuid::createUuid().toString(), QDate::currentDate().addDays(-3)}));
```
Returns a list of newly inserted ids. 
NOTE: this functional relies on "... RETURNING id;" feature support, my target was PostgreSQL. The `Database` class will try to determine the primary key, but if you *really mean something strange* another column can be specified instead, like `Query("my_table", "some_col")`. It is just a string, you can pass there whatever `RETURNING` supports, but a have not tested that option thorougly.

### Transactions

Every query is already a transaction of its own. `transact()` groups several queries into a single one:

```cpp
const Query query(targetTable);

const QString goodName {"GOOD_TRANSACTION"};
const QString badName {"BAD_TRANSACTION"};

auto committed = db.transact([&]{ // don't use default capture in real project
    const auto ids = db.execute(query
            .insert({"_otype", "guid", "name"})
            .values({33, QUuid::createUuid().toString(), goodName})
            .values({33, QUuid::createUuid().toString(), goodName}));
    Q_ASSERT(ids && ids->count() == 2);

    const auto deleted = db.execute(query.delete_(EQ("_id", ids->first())));
    Q_ASSERT(deleted && *deleted == 1);
});
Q_ASSERT(committed);

// The transaction has been commited, we can check that there's only one record on the table
const auto check1 = db.execute(query.select().where(EQ("name", goodName)));
Q_ASSERT(check1 && check1->count() == 1);
 ```

 ... and here goes a "bad" transaction, that's been rollbacked:

```cpp
committed = db.transact([&]{
    const auto ids = db.execute(query
            .insert({"_otype", "guid", "name"})
            .values({33, QUuid::createUuid().toString(), badName})
            .values({33, QUuid::createUuid().toString(), badName}));
    Q_ASSERT(ids && ids->count() == 2);

    const auto deleted = db.execute(query.delete_(EQ("_id_OOPS", ids->first())));
    Q_ASSERT(!deleted); // deleted.error() holds the error of the query
});
Q_ASSERT(!committed); // committed.error() holds the first error of the transaction

// Nothing's been inserted due to the error, after rollback we can check it
const auto check2 = db.execute(query.select().where(EQ("name", badName)));
Q_ASSERT(check2 && check2->isEmpty());
```

The transaction is rollbacked, if any of its queries failed, even if the later ones succeeded.
You can even write a *nested* `transact()`: it joins the outer transaction, only the outermost one commits or rollbacks everything.

### Select with JOINs

```cpp
const auto res = db.execute(query
            .select({"id", "some_text", "name", "guid"})
            .join("other_table_name", {"some_fkey", "id"}, Join::INNER)
            .orderBy("id", Order::ASC));

const auto res = db.execute(query
            .select()
            .join("other_table_name", {"some_fkey", "id"}, Join::INNER, true) // overriding disambiguation resolution
            .orderBy("id", Order::DESC));
```

Well, that is where the tricky part is. While you are having only two joined tables everything is pretty simple -- you specify the parameters and still don't nave to care about methods' call order.
If you are not carefull and column's name disambiguation occurs, it's gonna be resolved in favour of the table, specified in the `Query`'s  constructor by deafult. Another behaviour can be obtained via boolean flag.

But things are different with multiple joins. Actually, `join()` is the only method that doesn't overwrite the state being called more than once, accumulating JOIN parts instead. I cannot imagine from the top of my head WHY would anyone write SQL with multiple JOINs and column disambiguation at the same time, so here works a simple rule -- if you do it's either resolved by default or you *may* set a magic flag again... but wherever you do it, resolution's gonna work only in favour of the table in the first `join()` call, whatever it be.

```cpp
const auto res = db.execute(complexQuery
            .select({"_id", "some_text", "name", "guid", "some_date"})
            .join("second_table", {"some_fkey", "_id"}, Join::INNER, true)  // disambig can ONLY be resolved to this table, no matter where is the flag
            .join("third_table", {"date_fkey", "_id"}, Join::INNER, true)
            .where(IN("_id", joinIds))
            .orderBy("_id", Order::ASC));
```
Honestly you should not write queries like that even manually.

### Extras 

```cpp
const Query query("my_table");

auto res = db.execute(query.select({"*"})   // this means "SELECT * FROM ..." 
            .orderBy("_id", Order::ASC)
            .limit(5));

res = db.execute(query.select()   // this means "SELECT id, name, ... FROM ...", all column names are inserted  
            .orderBy("_id", Order::ASC)
            .limit(5));

res = db.execute(query.select({"COUNT(*) as count"})); // yeah, you can use functions
Q_ASSERT(res && !res->isEmpty());
Q_ASSERT(res->first().toMap().contains("count"));

res = db.execute(query.select({"MAX(_id) as max_id"})); // or even like that
```

There possible exist some other hacky ways to use the builder, I just did my best to make sure that the worst you'd get is a query error, returned by `Database` (exactly as Qt reports it, receiving from the database itself), all the rest builder classes and helpers are hopefully lightweight enought not to bring any unexpected trouble.

### Connection and transaction commands

`Query::beginTransaction()`, `Query::commit()` and `Query::rollback()` return SQL text only. They never execute statements, create a connection or commit a transaction on their own. Execute the resulting SQL on the connection whose transaction you intend to control; do not send COMMIT through `Database::performSQL()`, which manages its own transaction.

`Query::set(option, expression)` returns `SET <option> <expression>;`. Both arguments are trusted, dialect-specific SQL syntax, not bound user input. The expression is deliberately not quoted or escaped: SQL Server accepts `Query::set("LOCK_TIMEOUT", "30000")` and `Query::set("TRANSACTION ISOLATION LEVEL", "SERIALIZABLE")`. This permits keywords and numeric syntax without turning them into string literals. Applications must not pass unchecked external strings.

The `QSQLBUILDER_BUILD_TESTS` CMake option enables deterministic command-generation tests and registers them with CTest. It defaults to ON for a standalone build and OFF when included as a subdirectory.
