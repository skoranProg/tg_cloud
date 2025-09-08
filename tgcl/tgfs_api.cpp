#include "tgfs_api.h"
#include "../encryption/encrypt_file.h"
#include <cstdio>
#include <filesystem>

td_client_api::td_client_api(TdClass *client_, file_encryptor *encryptor_) : client_(client_), encryptor_(encryptor_) {
    std::string path = client_->GetDatabaseDir() + "/" + "a";
    if (download_table(path) == 3) {
        std::filesystem::remove(path);
        std::cout << "Incorrect key" << std::endl;
        exit(0);
    }
    current_table_id_ = -1;
    std::filesystem::remove(path);
}

int td_client_api::download(uint64_t msg, const std::string &path)  {
    auto fl = client_->DownloadFileFromMes(client_->GetMessage(client_->GetMainChatId(), msg));
    std::string encrypted_path = path + ".aes";
    int rename_res = rename(fl->local_->path_.c_str(), encrypted_path.c_str());
    int res = encryptor_->decrypt(encrypted_path, path);
    std::filesystem::remove(encrypted_path);
    client_->DeleteFile(fl->id_);
    return res;
}

int td_client_api::remove(uint64_t msg)  {
    client_->DeleteMessage(client_->GetMainChatId(), msg);
    return 0;
}

uint64_t td_client_api::upload(const std::string &path)  {
    std::string encrypted_path = path + ".aes";
    encryptor_->encrypt(path, encrypted_path);
    int file_id;
    uint64_t result = client_->SendFile(client_->GetMainChatId(), encrypted_path, &file_id);
    client_->DeleteFile(file_id);
    std::filesystem::remove(encrypted_path);
    return result;
}

int td_client_api::download_table(const std::string &path) {
    auto pinned = client_->GetLastPinnedMessage(client_->GetMainChatId());
    if (is_up_to_date_table()) {
        return 1;
    }
    if (pinned == nullptr) {
        return 2;
    }
    current_table_id_ = pinned->id_;
    int res = download(current_table_id_, path);
    if (res) {
        return 3;
    }
    return 0;
};

int td_client_api::upload_table(const std::string &path) {
    uint64_t id_ = upload(path);
    auto pinned = client_->GetLastPinnedMessage(client_->GetMainChatId());
    if (pinned) {
        remove(pinned->id_);
    }
    client_->PinMessage(client_->GetMainChatId(), id_);
    current_table_id_ = id_;
    return 0;
}

bool td_client_api::is_up_to_date_table() {
    auto pinned = client_->GetLastPinnedMessage(client_->GetMainChatId());
    if (pinned == nullptr) {
        return false;
    }
    return pinned->id_ == current_table_id_;
}
