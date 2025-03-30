#include "td_helpers.h"

std::unordered_map<fuse_ino_t, uint64_t> TableLoader::LoadTable() {
    std::unordered_map<fuse_ino_t, uint64_t> result;
    const std::string sql = "SELECT * from DIRECTORIES";
    int rc = sqlite3_exec(db_, sql.c_str(), [](void *data, int argc, char **argv, char **azColName) {
        uint64_t first_arg = std::stoll(argv[0]);
        static_cast<std::unordered_map<fuse_ino_t, uint64_t>*>(data)->operator[](first_arg) = std::stoll(argv[1]);
        return 0;
    }, &result, nullptr);
    return result;
}

void TableLoader::LoadInTable(std::unordered_map<fuse_ino_t, uint64_t>& table) {
    //ClearTable();
    auto gen_insert = [](fuse_ino_t ino, uint64_t mes_id) {
        return  "INSERT INTO DIRECTORIES (INO, MES_ID) VALUES (" + std::to_string(ino) + ", " + std::to_string(mes_id) + ");";
    };
    std::string sql;
    for (auto & [ino, mes_id] : table) {
        sql += gen_insert(ino, mes_id);
    }
    sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, nullptr);
}

void TableLoader::CreateSqlDB() {
    const std::string sql = "CREATE TABLE DIRECTORIES("  \
  "INO bigint," \
  "MES_ID bigint);";
    sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, nullptr);
}

void TableLoader::ClearTable() {
    sqlite3_db_config(db_, SQLITE_DBCONFIG_RESET_DATABASE, 1, 0);
    sqlite3_exec(db_, "VACUUM", 0, 0, 0);
    sqlite3_db_config(db_, SQLITE_DBCONFIG_RESET_DATABASE, 0, 0);
}