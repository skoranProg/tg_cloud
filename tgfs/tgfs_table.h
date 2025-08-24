#ifndef _TGFS_TABLE_H_
#define _TGFS_TABLE_H_

#include "../SQLiteCpp/sqlite3/sqlite3.h"
#include <concepts>
#include <format>
#include <stdint.h>
#include <string>

class tgfs_db {
  public:
    sqlite3 *table_;

    explicit tgfs_db(const std::string &path);
    ~tgfs_db();
};

template <class K>
concept IntOrStr =
    std::is_integral<K>::value || std::is_same<K, std::string>::value;

template <IntOrStr K, std::integral V> class tgfs_table : public tgfs_db {

  public:
    explicit tgfs_table(const std::string &path) : tgfs_db{path} {}

    int init();

    V at(K key);
    bool contains(K key);
    int set(K key, V value);
    int remove(K key);
};

#include "tgfs_table_impl.h"

#endif
