#include "tgfs_table.h"

tgfs_db::tgfs_db(const std::string &path) {
    sqlite3_open(path.c_str(), &table_);
}

tgfs_db::~tgfs_db() {
    sync();
    sqlite3_close(table_);
}

void tgfs_db::sync() {
    sqlite3_db_cacheflush(table_);
}
