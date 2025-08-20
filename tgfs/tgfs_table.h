#ifndef _TGFS_TABLE_H_
#define _TGFS_TABLE_H_

#include "../SQLiteCpp/sqlite3/sqlite3.h"
#include <concepts>
#include <stdint.h>
#include <string>

class tgfs_db {
  public:
    sqlite3 *table_;

    explicit tgfs_db(const std::string &path);
    ~tgfs_db();
};

template <class K, class V> class tgfs_table;

template <std::integral K, std::integral V>
class tgfs_table<K, V> : public tgfs_db {

  public:
    explicit tgfs_table(const std::string &path) : tgfs_db{path} {}

    int init();

    V at(K key);
    bool contains(K key);
    int set(K key, V value);
    int remove(K key);
};

template <std::integral V> class tgfs_table<std::string, V> : public tgfs_db {

  public:
    explicit tgfs_table(const std::string &path) : tgfs_db{path} {}

    int init();

    V at(std::string key);
    bool contains(std::string key);
    int set(std::string key, V value);
    int remove(std::string key);
};

#endif
