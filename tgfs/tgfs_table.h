#ifndef _TGFS_TABLE_H_
#define _TGFS_TABLE_H_

#include "../SQLiteCpp/sqlite3/sqlite3.h"
#include <concepts>
#include <stdint.h>
#include <string>

template <std::integral K, std::integral V> class tgfs_table {
  private:
    sqlite3 *table_;

  public:
    explicit tgfs_table(const std::string &path);
    ~tgfs_table();

    int init();

    static const V VZERO;

    V at(K key);
    bool contains(K key);
    int set(K key, V value);
    int remove(K key);
};

template <std::integral K, std::integral V> const V tgfs_table<K, V>::VZERO = 0;

#endif
