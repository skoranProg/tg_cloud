#ifndef TD_HELPER_H
#define TD_HELPER_H

#include <iostream>
#include <string.h>
#include <sqlite3.h>
#include <unordered_map>
#include <filesystem>
#include "tgfs/tgfs_fuse_dependencies.h"
#include "tdclient.h"

class TableLoader {
public:
    explicit TableLoader(sqlite3 *db) : db_(db) {
    }

    std::unordered_map<fuse_ino_t, uint64_t> LoadTable();

    void LoadInTable(std::unordered_map<fuse_ino_t, uint64_t>& table);

    void CreateSqlDB();

    void ClearTable();
private:

    sqlite3 *db_;

};


#endif //TD_HELPER_H
