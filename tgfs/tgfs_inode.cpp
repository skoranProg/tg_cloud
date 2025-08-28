#include "tgfs_inode.h"

#include <format>

tgfs_inode::tgfs_inode(struct stat attr, uint64_t version)
    : attr{attr}, version{version}, data_msg_{0}, data_version_{0} {}

int tgfs_inode::update_data(tgfs_net_api *api, int n,
                            const std::string &root_path) {
    if (n != 0) {
        return 1;
    }
    if (data_msg_ == data_version_) {
        return 0;
    }
    api->download(data_msg_, std::format("{}{}/{}", root_path, attr.st_ino, n));
    return 0;
}

int tgfs_inode::upload_data(tgfs_net_api *api, int n,
                            const std::string &root_path) {
    if (n != 0) {
        return 1;
    }
    data_version_ =
        api->upload(std::format("{}{}/{}", root_path, attr.st_ino, n));
    if (data_msg_ != 0) {
        api->remove(data_msg_);
    }
    data_msg_ = data_version_;
    return 0;
}
