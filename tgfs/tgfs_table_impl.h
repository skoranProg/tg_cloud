#ifndef _TGFS_TABLE_IMPL_H_
#define _TGFS_TABLE_IMPL_H_

#include <format>
#include <iostream>

#include "tgfs_table.h"

template <IntOrStr T>
struct tgfs_sql_key {
    T key;
};

namespace std {

template <>
struct formatter<tgfs_sql_key<string>, char> : formatter<std::string, char> {
    // template <class ParseContext>
    // constexpr ParseContext::iterator parse(ParseContext &ctx) {
    //     return ctx.end();
    // }

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

}  // namespace std

template <IntOrStr K, std::integral V>
int tgfs_table<K, V>::init() {
    char *err = nullptr;
    std::clog << "DB init() !!! " << table_ << std::endl;
    sqlite3_exec(table_,
                 std::format("CREATE TABLE my_table ( "
                             "my_key {} PRIMARY KEY, "
                             "my_value INTEGER "
                             ");",
                             (std::is_integral<K>::value) ? "INTEGER" : "TEXT")
                     .c_str(),
                 nullptr, nullptr, &err);
    std::clog << "\tcomplete!" << std::endl;
    if (err) {
        std::clog << err << std::endl;
        sqlite3_free(err);
    }
    return 0;
}

template <IntOrStr K, std::integral V>
V tgfs_table<K, V>::at(K key) {
    V res = 0;
    char *err = nullptr;
    std::clog << "DB at() !!!  " << std::format("{}", tgfs_sql_key<K>{key})
              << std::endl;
    sqlite3_exec(
        table_,
        std::format("SELECT my_value FROM my_table WHERE my_key = {} LIMIT 1;",
                    tgfs_sql_key<K>{key})
            .c_str(),
        [](void *res, int n, char *values[], char *columns[]) {
            *reinterpret_cast<V *>(res) = static_cast<V>(std::atoll(values[0]));
            return 0;
        },
        &res, &err);
    std::clog << "\tresult:  " << res << std::endl;
    if (err) {
        std::clog << err << std::endl;
        sqlite3_free(err);
    }
    return res;
}

template <IntOrStr K, std::integral V>
bool tgfs_table<K, V>::contains(K key) {
    bool res = false;
    char *err = nullptr;
    std::clog << "DB contains() !!!  "
              << std::format("{}", tgfs_sql_key<K>{key}) << std::endl;
    sqlite3_exec(
        table_,
        std::format("SELECT my_key FROM my_table WHERE my_key = {} LIMIT 1;",
                    tgfs_sql_key<K>{key})
            .c_str(),
        [](void *res, int n, char *values[], char *columns[]) {
            *reinterpret_cast<bool *>(res) = true;
            return 0;
        },
        &res, &err);
    std::clog << "\tresult: " << (res ? "true" : "false") << std::endl;
    if (err) {
        std::clog << err << std::endl;
        sqlite3_free(err);
    }
    return res;
}

template <IntOrStr K, std::integral V>
int tgfs_table<K, V>::set(K key, V value) {
    char *err = nullptr;
    std::clog << "DB insert() !!!  " << std::format("{}", tgfs_sql_key<K>{key})
              << ' ' << value << std::endl;
    sqlite3_exec(
        table_,
        std::format(
            "INSERT INTO my_table (my_key, my_value) VALUES ({}, {}) ON "
            "CONFLICT(my_key) DO UPDATE SET my_value=excluded.my_value;",
            tgfs_sql_key<K>{key}, value)
            .c_str(),
        nullptr, nullptr, &err);
    std::clog << "\tinserted!" << std::endl;
    if (err) {
        std::clog << err << std::endl;
        sqlite3_free(err);
    }
    return 0;
}

template <IntOrStr K, std::integral V>
int tgfs_table<K, V>::remove(K key) {
    char *err = nullptr;
    std::clog << "DB remove() !!!  " << std::format("{}", tgfs_sql_key<K>{key})
              << std::endl;
    sqlite3_exec(table_,
                 std::format("DELETE FROM my_table WHERE my_key = {};",
                             tgfs_sql_key<K>{key})
                     .c_str(),
                 nullptr, nullptr, &err);
    std::clog << "\tremoved!" << std::endl;
    if (err) {
        std::clog << err << std::endl;
        sqlite3_free(err);
    }
    return 0;
}

template <IntOrStr K, std::integral V>
K tgfs_table<K, V>::max_key() {
    static_assert(std::is_integral<K>::value,
                  "Function max_key() makes sense only for integral keys!");
    K res = 0;
    char *err = nullptr;
    std::clog << "DB max() !!!  " << std::endl;
    sqlite3_exec(
        table_, "SELECT MAX(my_key) FROM my_table;",
        [](void *res, int n, char *values[], char *columns[]) {
            *reinterpret_cast<K *>(res) = static_cast<K>(std::atoll(values[0]));
            return 0;
        },
        &res, &err);
    std::clog << "\tresult:  " << res << std::endl;
    if (err) {
        std::clog << err << std::endl;
        sqlite3_free(err);
    }
    return res;
}

#endif
