#ifndef _TGFS_TABLE_IMPL_H_
#define _TGFS_TABLE_IMPL_H_

#include "tgfs_table.h"
#include <format>

template <IntOrStr T> struct tgfs_sql_key {
    T key;
};

namespace std {

template <> struct formatter<tgfs_sql_key<string>, char> {

    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext &ctx) {
        return ctx.end();
    }

    template <class FmtContext>
    FmtContext::iterator format(tgfs_sql_key<string> val,
                                FmtContext &ctx) const {
        return format_to(ctx.out(), "\'{}\'", val.key);
    }
};

template <integral T>
struct formatter<tgfs_sql_key<T>, char> : formatter<T, char> {

    template <class FmtContext>
    FmtContext::iterator format(tgfs_sql_key<T> val, FmtContext &ctx) const {
        return formatter<T, char>::format(val.key, ctx);
    }
};

} // namespace std

#pragma mmap_size = 268435456;

template <IntOrStr K, std::integral V> int tgfs_table<K, V>::init() {
    char *err;
    sqlite3_exec(table_,
                 std::format("CREATE TABLE my_table ("
                             "my_key {} PRIMARY KEY"
                             "my_value INTEGER"
                             ");",
                             (std::is_integral<K>::value) ? "INTEGER" : "TEXT")
                     .c_str(),
                 nullptr, nullptr, &err);
    if (err) {
        sqlite3_free(err);
    }
    return 0;
}

template <IntOrStr K, std::integral V> V tgfs_table<K, V>::at(K key) {
    V res = 0;
    char *err;
    sqlite3_exec(
        table_,
        std::format("SELECT my_value FROM my_table WHERE my_key = {};",
                    tgfs_sql_key<K>{key})
            .c_str(),
        [](void *res, int n, char *values[], char *columns[]) {
            *reinterpret_cast<V *>(res) = *reinterpret_cast<V *>(values[0]);
            return 1;
        },
        &res, &err);
    if (err) {
        sqlite3_free(err);
    }
    return res;
}

template <IntOrStr K, std::integral V> bool tgfs_table<K, V>::contains(K key) {
    bool res = false;
    char *err;
    sqlite3_exec(
        table_,
        std::format("SELECT my_key FROM my_table WHERE my_key = {};",
                    tgfs_sql_key<K>{key})
            .c_str(),
        [](void *res, int n, const char *values[], const char *columns[]) {
            *reinterpret_cast<bool *>(res) = true;
            return 1;
        },
        &res, &err);
    if (err) {
        sqlite3_free(err);
    }
    return res;
}

template <IntOrStr K, std::integral V>
int tgfs_table<K, V>::set(K key, V value) {
    char *err;
    sqlite3_exec(
        table_,
        std::format(
            "INSERT INTO my_table (my_key, my_value) VALUES ({}, {}) ON "
            "CONFLICT(my_key) DO UPDATE SET my_value=excluded.my_value;",
            tgfs_sql_key<K>{key}, value)
            .c_str(),
        nullptr, nullptr, &err);
    if (err) {
        sqlite3_free(err);
    }
    return 0;
}

template <IntOrStr K, std::integral V> int tgfs_table<K, V>::remove(K key) {
    char *err;
    sqlite3_exec(table_,
                 std::format("DELETE FROM my_table WHERE my_key = {};",
                             tgfs_sql_key<K>{key})
                     .c_str(),
                 nullptr, nullptr, &err);
    if (err) {
        sqlite3_free(err);
    }
    return 0;
}

#endif
