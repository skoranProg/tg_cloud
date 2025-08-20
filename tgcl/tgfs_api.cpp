#include "tgfs_api.h"
#include <stdio.h>

int td_client_api::download(uint64_t msg, const std::string &path)  {
    auto fl = client_->DownloadFileFromMes(client_->GetMessage(client_->GetMainChatId(), msg));
    return rename(fl->local_->path_.c_str(),path.c_str());
}

int td_client_api::remove(uint64_t msg)  {
    client_->DeleteMessage(client_->GetMainChatId(), msg);
    return 0;
}

uint64_t td_client_api::upload(const std::string &path)  {
    return client_->SendFile(client_->GetMainChatId(), path);
}

int td_client_api::download_table(const std::string &path) {
    current_table_id_ = client_->GetLastPinnedMessage(client_->GetMainChatId())->id_;
    download(current_table_id_, path);
    return 0;
};

int td_client_api::upload_table(const std::string &path) {
    uint64_t id_ = upload(path);
    client_->PinMessage(client_->GetMainChatId(), id_);
    return 0;
}

bool td_client_api::is_up_to_date_table() {
    return client_->GetLastPinnedMessage(client_->GetMainChatId())->id_ == current_table_id_;
}
