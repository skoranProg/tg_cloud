#include "tgfs_table.h"

tgfs_db::tgfs_db(const std::string &path) {
    sqlite3_open(path.c_str(), &table_);
}

tgfs_db::~tgfs_db() { sqlite3_close(table_); }
