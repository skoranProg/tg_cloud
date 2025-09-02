#include "tgfs_inode.h"

#include <unistd.h>

#include <format>
#include <iostream>

tgfs_inode::tgfs_inode(struct stat attr, uint64_t version)
    : attr{attr}, version{version}, data_msg_{0}, data_version_{0} {}

int tgfs_inode::update_data(tgfs_net_api *api, int n,
                            const std::string &root_path) {
    std::clog << "tgfs_inode::update_data\n\tino: " << attr.st_ino
              << "\n\tsize: " << attr.st_size << "\n\tfile number: " << n
              << std::endl;
    if (attr.st_size == 0) {
        close(open(std::format("{}{}/data_0", root_path, attr.st_ino).c_str(),
                   O_CREAT | O_RDWR | O_TRUNC, S_IFREG | 0666));
        return 0;
    }
    if (n != 0) {
        return 1;
    }
    if (data_msg_ == data_version_) {
        return 0;
    }
    api->download(data_msg_,
                  std::format("{}{}/data_{}", root_path, attr.st_ino, n));
    data_version_ = data_msg_;
    return 0;
}

int tgfs_inode::upload_data(tgfs_net_api *api, int n,
                            const std::string &root_path) {
    std::clog << "tgfs_inode::upload_data\n\tino: " << attr.st_ino
              << "\n\tsize: " << attr.st_size << "\n\tfile number: " << n
              << std::endl;
    if (attr.st_size == 0) {
        return 0;
    }
    if (n != 0) {
        return 1;
    }
    data_version_ =
        api->upload(std::format("{}{}/data_{}", root_path, attr.st_ino, n));
    if (data_msg_ != 0) {
        api->remove(data_msg_);
    }
    data_msg_ = data_version_;
    return 0;
}
