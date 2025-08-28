#pragma once

#include "../tgfs/tgfs.h"
#include "tdclient.h"
#include "../encryption/encrypt_file.h"

class td_client_api : public tgfs_net_api {
public:
    td_client_api(TdClass* client_, file_encryptor* encryptor_) : client_(client_), encryptor_(encryptor_) {
    }

    uint64_t upload(const std::string &path) override;
    int download(uint64_t msg, const std::string &path) override;
    int remove(uint64_t msg) override;

    bool is_up_to_date_table() override;
    // Should return:
    // 0 - on successful download
    // 1 - if table already was up-to-date
    // 2 - if chat is empty
    // anything else - on error
    int download_table(const std::string &path) override;
    int upload_table(const std::string &path) override;
private:
    TdClass* client_;

    file_encryptor* encryptor_;

    uint64_t current_table_id_;
};

