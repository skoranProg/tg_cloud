#include "tgfs_table.h"
#include <format>

#pragma mmap_size = 268435456;

tgfs_db::tgfs_db(const std::string &path) {
    sqlite3_open(path.c_str(), &table_);
}

tgfs_db::~tgfs_db() { sqlite3_close(table_); }

// K = std::integral
// V = std::integral

template <std::integral K, std::integral V> int tgfs_table<K, V>::init() {
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

template <std::integral K, std::integral V> V tgfs_table<K, V>::at(K key) {
    V res = 0;
    char *err;
    sqlite3_exec(
        table_,
        std::format("SELECT my_value FROM my_table WHERE my_key = {};", key)
            .c_str(),
        [](void *res, int n, const char *values[], const char *columns[]) {
            *reinterpret_cast<V *>(res) = *reinterpret_cast<V *>(values[0]);
            return 1;
        },
        &res, &err);
    if (err) {
        sqlite3_free(err);
    }
    return res;
}

template <std::integral K, std::integral V>
bool tgfs_table<K, V>::contains(K key) {
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

template <std::integral K, std::integral V>
int tgfs_table<K, V>::set(K key, V value) {
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

template <std::integral K, std::integral V>
int tgfs_table<K, V>::remove(K key) {
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

// K = std::string
// V = std::integral

template <std::integral V> int tgfs_table<std::string, V>::init() {
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

template <std::integral V> V tgfs_table<std::string, V>::at(std::string key) {
    V res = 0;
    char *err;
    sqlite3_exec(
        table_,
        std::format("SELECT my_value FROM my_table WHERE my_key = \'{}\';", key)
            .c_str(),
        [](void *res, int n, const char *values[], const char *columns[]) {
            *reinterpret_cast<V *>(res) = *reinterpret_cast<V *>(values[0]);
            return 1;
        },
        &res, &err);
    if (err) {
        sqlite3_free(err);
    }
    return res;
}

template <std::integral V>
bool tgfs_table<std::string, V>::contains(std::string key) {
    bool res = false;
    char *err;
    sqlite3_exec(
        table_,
        std::format("SELECT my_key FROM my_table WHERE my_key = \'{}\';", key)
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

template <std::integral V>
int tgfs_table<std::string, V>::set(std::string key, V value) {
    char *err;
    sqlite3_exec(
        table_,
        std::format(
            "INSERT INTO my_table (my_key, my_value) VALUES (\'{}\', {}) ON "
            "CONFLICT(my_key) DO UPDATE SET my_value=excluded.my_value;",
            key, value)
            .c_str(),
        nullptr, nullptr, &err);
    if (err) {
        sqlite3_free(err);
    }
    return 0;
}

template <std::integral V>
int tgfs_table<std::string, V>::remove(std::string key) {
    char *err;
    sqlite3_exec(
        table_,
        std::format("DELETE FROM my_table WHERE my_key = \'{}\';", key).c_str(),
        nullptr, nullptr, &err);
    if (err) {
        sqlite3_free(err);
    }
    return 0;
}
