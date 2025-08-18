#ifndef _TGFS_TABLE_H_
#define _TGFS_TABLE_H_

#include "../SQLiteCpp/sqlite3/sqlite3.h"
#include "stdint.h"
#include <concepts>
#include <string>

template <std::integral K, std::integral V> class tgfs_table {
  private:
    sqlite3 *table_;

  public:
    explicit tgfs_table(const std::string &path);
    ~tgfs_table();

    int init();

    V at(K key);
    bool contains(K key);
    int set(K key, V value);
    int remove(K key);
};

#endif
