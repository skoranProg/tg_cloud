#include "tgfs_table.h"

#include <iostream>

tgfs_db::tgfs_db(const std::string &path) {
    int err = sqlite3_open(path.c_str(), &table_);
    sqlite3_exec(table_, "PRAGMA mmap_size=268435456;", nullptr, nullptr,
                 nullptr);
    // sqlite3_exec(table_, "PRAGMA synchronous=FULL;", nullptr, nullptr,
    // nullptr);
    std::cerr << "DB open() !!! " << err << "  " << table_ << " " << path
              << std::endl;
}

tgfs_db::~tgfs_db() {
    sync();
    sqlite3_close(table_);
    std::cerr << "DB close() !!!" << std::endl;
}

void tgfs_db::sync() {
    // int err0 =
    //     sqlite3_wal_checkpoint_v2(table_, "main", SQLITE_CHECKPOINT_FULL, 0,
    //     0);
    int err1 = sqlite3_db_cacheflush(table_);
    // int err2 =
    //     sqlite3_file_control(table_, nullptr, SQLITE_FCNTL_SYNC, nullptr);
    std::cerr << "DB sync() !!!" /*<< "  " << err0 */ << "  "
              << err1 /*<< "  " << err2*/
              << std::endl;
}
