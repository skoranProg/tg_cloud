#include "tgfs_inode.h"

#include <sys/mman.h>
#include <unistd.h>

#include <format>
#include <iostream>

tgfs_inode::tgfs_inode(fuse_ino_t ino, const std::string &root_path)
    : attr{nullptr},
      nlookup{0},
      version{0},
      need_datasync_{true},
      data_version_{0} {
    int fd = open(std::format("{}/{}/inode", root_path, ino).c_str(),
                  O_CREAT | O_RDWR, S_IFREG | 0666);
    posix_fallocate(fd, 0, sizeof(persistent_data));
    attr = reinterpret_cast<persistent_data *>(
        mmap(NULL, sizeof(persistent_data), PROT_READ | PROT_WRITE, MAP_SHARED,
             fd, 0));
    close(fd);
}

tgfs_inode::~tgfs_inode() {
    munmap(attr, sizeof(persistent_data));
}

int tgfs_inode::update_data(tgfs_net_api *api, int n,
                            const std::string &root_path) {
    std::clog << "tgfs_inode::update_data\n\tino: " << attr->st_ino
              << "\n\tsize: " << attr->st_size << "\n\tfile number: " << n
              << std::endl;
    if (attr->st_size == 0) {
        close(open(std::format("{}/{}/data_0", root_path, attr->st_ino).c_str(),
                   O_CREAT | O_RDWR | O_TRUNC, S_IFREG | 0666));
        return 0;
    }
    if (n != 0) {
        return 1;
    }
    if (attr->data_msg_ == data_version_) {
        return 0;
    }
    api->download(attr->data_msg_,
                  std::format("{}/{}/data_{}", root_path, attr->st_ino, n));
    data_version_ = attr->data_msg_;
    need_datasync_ = false;
    return 0;
}

int tgfs_inode::upload_data(tgfs_net_api *api, int n,
                            const std::string &root_path) {
    std::clog << "tgfs_inode::upload_data\n\tino: " << attr->st_ino
              << "\n\tsize: " << attr->st_size << "\n\tfile number: " << n
              << "\n\tdatasync: " << (need_datasync_ ? "true" : "false")
              << std::endl;
    if (!need_datasync_ || attr->st_size == 0) {
        return 0;
    }
    if (n != 0) {
        return 1;
    }
    data_version_ =
        api->upload(std::format("{}/{}/data_{}", root_path, attr->st_ino, n));
    remove_data(api);
    attr->data_msg_ = data_version_;
    need_datasync_ = false;
    return 0;
}

void tgfs_inode::remove_data(tgfs_net_api *api) {
    if (attr->data_msg_ != 0) {
        api->remove(attr->data_msg_);
    }
}

void tgfs_inode::datawrite() {
    need_datasync_ = true;
}
