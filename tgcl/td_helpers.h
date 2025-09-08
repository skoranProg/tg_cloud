#ifndef TD_HELPER_H
#define TD_HELPER_H

#include <../SQLiteCpp/sqlite3/sqlite3.h>
#include <string.h>

#include <filesystem>
#include <iostream>
#include <unordered_map>

#include "../tgfs/tgfs_fuse_dependencies.h"
#include "tdclient.h"

class TableLoader {
 public:
    explicit TableLoader(sqlite3 *db) : db_(db) {}

    std::unordered_map<fuse_ino_t, uint64_t> LoadTable();

    void LoadInTable(std::unordered_map<fuse_ino_t, uint64_t> &table);

    void CreateSqlDB();

    void ClearTable();

 private:
    sqlite3 *db_;
};

class TableTdUpdater {
 public:
    explicit TableTdUpdater(TdClass *td_client) : td_client_(td_client) {}

    int upload_table(std::unordered_map<fuse_ino_t, uint64_t> &messages) const;

    std::unordered_map<fuse_ino_t, uint64_t> get_table() const;

 private:
    TdClass *td_client_;
};

// mes must contain MessageDocument

td::tl_object_ptr<td_api::file> DownloadFileFromMes(
    TdClass *td_class, td::tl_object_ptr<td_api::message> mes);

#endif  // TD_HELPER_H
