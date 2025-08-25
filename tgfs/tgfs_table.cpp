#include "tgfs_table.h"

#include <iostream>

tgfs_db::tgfs_db(const std::string &path) {
    sqlite3_open(path.c_str(), &table_);
    sqlite3_exec(table_, "PRAGMA mmap_size=268435456;", nullptr, nullptr,
                 nullptr);
    std::cerr << "DB open() !!!" << std::endl;
}

tgfs_db::~tgfs_db() {
    sync();
    sqlite3_close(table_);
    std::cerr << "DB close() !!!" << std::endl;
}

void tgfs_db::sync() {
    sqlite3_db_cacheflush(table_);
    std::cerr << "DB sync() !!!" << std::endl;
}
