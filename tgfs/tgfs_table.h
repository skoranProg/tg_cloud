#ifndef _TGFS_TABLE_H_
#define _TGFS_TABLE_H_

#include "../SQLiteCpp/sqlite3/sqlite3.h"
#include "stdint.h"
#include <string>

template <class K, class V> class tgfs_table {
  private:
    sqlite3 *table_;
    uint64_t version;

    int update();
    int upload();

  public:
    explicit tgfs_table(std::string path);
    V at(K key);
    bool contains(K key);
    int set(K key, V value);
};

#endif
