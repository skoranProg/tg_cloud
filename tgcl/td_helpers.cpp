#include "td_helpers.h"

std::unordered_map<fuse_ino_t, uint64_t> TableLoader::LoadTable() {
    std::unordered_map<fuse_ino_t, uint64_t> result;
    const std::string sql = "SELECT * from DIRECTORIES";
    int rc = sqlite3_exec(
        db_, sql.c_str(),
        [](void *data, int argc, char **argv, char **azColName) {
            uint64_t first_arg = std::stoll(argv[0]);
            static_cast<std::unordered_map<fuse_ino_t, uint64_t> *>(data)
                ->operator[](first_arg) = std::stoll(argv[1]);
            return 0;
        },
        &result, nullptr);
    return result;
}

void TableLoader::LoadInTable(std::unordered_map<fuse_ino_t, uint64_t> &table) {
    // ClearTable();
    auto gen_insert = [](fuse_ino_t ino, uint64_t mes_id) {
        return "INSERT INTO DIRECTORIES (INO, MES_ID) VALUES (" +
               std::to_string(static_cast<int64_t>(ino)) + ", " +
               std::to_string(static_cast<int64_t>(mes_id)) + ");";
    };
    std::string sql;
    for (auto &[ino, mes_id] : table) {
        sql += gen_insert(ino, mes_id);
    }
    sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, nullptr);
}

void TableLoader::CreateSqlDB() {
    const std::string sql =
        "CREATE TABLE DIRECTORIES("
        "INO bigint,"
        "MES_ID bigint);";
    sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, nullptr);
}

void TableLoader::ClearTable() {
    sqlite3_db_config(db_, SQLITE_DBCONFIG_RESET_DATABASE, 1, 0);
    sqlite3_exec(db_, "VACUUM", 0, 0, 0);
    sqlite3_db_config(db_, SQLITE_DBCONFIG_RESET_DATABASE, 0, 0);
}

td::tl_object_ptr<td_api::file> DownloadFileFromMes(
    TdClass *td_class, td::tl_object_ptr<td_api::message> mes) {
    auto fl = td::move_tl_object_as<td_api::messageDocument>(mes->content_);
    return td_class->DownloadFile(fl->document_->document_->id_);
}

int TableTdUpdater::upload_table(
    std::unordered_map<fuse_ino_t, uint64_t> &messages) const {
    /// TODO: handle errors

    sqlite3 *db;
    sqlite3_open("tmp.db", &db);
    TableLoader loader(db);
    loader.CreateSqlDB();
    loader.LoadInTable(messages);
    auto res = loader.LoadTable();
    sqlite3_close(db);
    td_client_->SendFile(td_client_->GetMainChatId(), "tmp.db");
    td_client_->PinMessage(
        td_client_->GetMainChatId(),
        td_client_->GetLastMessage(td_client_->GetMainChatId())->id_);
    std::filesystem::remove("tmp.db");
    return 0;
}

std::unordered_map<fuse_ino_t, uint64_t> TableTdUpdater::get_table() const {
    /// TODO: handle errors

    auto last_mes =
        td_client_->GetLastPinnedMessage(td_client_->GetMainChatId());
    auto td_file =
        td::move_tl_object_as<td_api::messageDocument>(last_mes->content_);
    auto dw_file = td_client_->DownloadFile(td_file->document_->document_->id_);
    std::string path = dw_file->local_->path_;
    sqlite3 *db;
    sqlite3_open(path.c_str(), &db);
    TableLoader loader(db);
    auto messages = loader.LoadTable();
    sqlite3_close(db);
    std::filesystem::remove(dw_file->local_->path_);

    return messages;
}
