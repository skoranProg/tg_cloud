#ifndef _TGFS_TABLE_H_
#define _TGFS_TABLE_H_

#include "../SQLiteCpp/sqlite3/sqlite3.h"
#include <concepts>
#include <format>
#include <stdint.h>
#include <string>

#pragma mmap_size = 268435456;

class tgfs_db {
  public:
    sqlite3 *table_;

    explicit tgfs_db(const std::string &path);
    ~tgfs_db();
};

using sqlite3_callback = int (*)(void *, int, char **, char **);

template <class K, class V> class tgfs_table;

template <std::integral K, std::integral V>
class tgfs_table<K, V> : public tgfs_db {

  public:
    explicit tgfs_table(const std::string &path) : tgfs_db{path} {}

    int init() {
        char *err;
        sqlite3_exec(table_,
                     std::format("CREATE TABLE my_table ("
                                 "my_key INTEGER PRIMARY KEY"
                                 "my_value INTEGER"
                                 ");")
                         .c_str(),
                     nullptr, nullptr, &err);
        if (err) {
            sqlite3_free(err);
        }
        return 0;
    }

    V at(K key) {
        V res = 0;
        char *err;
        sqlite3_exec(
            table_,
            std::format("SELECT my_value FROM my_table WHERE my_key = {};", key)
                .c_str(),
            [](void *res, int n, char *values[], char *columns[]) {
                *reinterpret_cast<V *>(res) =
                    *reinterpret_cast<const V *>(values[0]);
                return 1;
            },
            &res, &err);
        if (err) {
            sqlite3_free(err);
        }
        return res;
    }

    bool contains(K key) {
        bool res = false;
        char *err;
        sqlite3_exec(
            table_,
            std::format("SELECT my_key FROM my_table WHERE my_key = {};", key)
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

    int set(K key, V value) {
        char *err;
        sqlite3_exec(
            table_,
            std::format(
                "INSERT INTO my_table (my_key, my_value) VALUES ({}, {}) ON "
                "CONFLICT(my_key) DO UPDATE SET my_value=excluded.my_value;",
                key, value)
                .c_str(),
            nullptr, nullptr, &err);
        if (err) {
            sqlite3_free(err);
        }
        return 0;
    }

    int remove(K key) {
        char *err;
        sqlite3_exec(
            table_,
            std::format("DELETE FROM my_table WHERE my_key = {};", key).c_str(),
            nullptr, nullptr, &err);
        if (err) {
            sqlite3_free(err);
        }
        return 0;
    }
};

template <std::integral V> class tgfs_table<std::string, V> : public tgfs_db {

  public:
    explicit tgfs_table(const std::string &path) : tgfs_db{path} {}

    int init() {
        char *err;
        sqlite3_exec(table_,
                     std::format("CREATE TABLE my_table ("
                                 "my_key TEXT PRIMARY KEY"
                                 "my_value INTEGER"
                                 ");")
                         .c_str(),
                     nullptr, nullptr, &err);
        if (err) {
            sqlite3_free(err);
        }
        return 0;
    }

    V at(std::string key) {
        V res = 0;
        char *err;
        sqlite3_exec(
            table_,
            std::format("SELECT my_value FROM my_table WHERE my_key = \'{}\';",
                        key)
                .c_str(),
            [](void *res, int n, const char *values[], const char *columns[]) {
                *reinterpret_cast<V *>(res) =
                    *reinterpret_cast<const V *>(values[0]);
                return 1;
            },
            &res, &err);
        if (err) {
            sqlite3_free(err);
        }
        return res;
    }

    bool contains(std::string key) {
        bool res = false;
        char *err;
        sqlite3_exec(
            table_,
            std::format("SELECT my_key FROM my_table WHERE my_key = \'{}\';",
                        key)
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

    int set(std::string key, V value) {
        char *err;
        sqlite3_exec(
            table_,
            std::format(
                "INSERT INTO my_table (my_key, my_value) VALUES (\'{}\', {}) "
                "ON "
                "CONFLICT(my_key) DO UPDATE SET my_value=excluded.my_value;",
                key, value)
                .c_str(),
            nullptr, nullptr, &err);
        if (err) {
            sqlite3_free(err);
        }
        return 0;
    }

    int remove(std::string key) {
        char *err;
        sqlite3_exec(
            table_,
            std::format("DELETE FROM my_table WHERE my_key = \'{}\';", key)
                .c_str(),
            nullptr, nullptr, &err);
        if (err) {
            sqlite3_free(err);
        }
        return 0;
    }
};

#endif
