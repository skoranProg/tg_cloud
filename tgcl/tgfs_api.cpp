#include "tgfs_api.h"
#include "../encryption/encrypt_file.h"
#include <stdio.h>

int td_client_api::download(uint64_t msg, const std::string &path)  {
    auto fl = client_->DownloadFileFromMes(client_->GetMessage(client_->GetMainChatId(), msg));
    int rename_res = rename(fl->local_->path_.c_str(),path.c_str());
    if (!rename_res) {
        return rename_res;
    }
    return encryptor_->encrypt(path, "");
}

int td_client_api::remove(uint64_t msg)  {
    client_->DeleteMessage(client_->GetMainChatId(), msg);
    return 0;
}

uint64_t td_client_api::upload(const std::string &path)  {
    encryptor_->encrypt(path, "");
    uint64_t result = client_->SendFile(client_->GetMainChatId(), path);
    encryptor_->decrypt(path, "");
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
    download(current_table_id_, path);
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
