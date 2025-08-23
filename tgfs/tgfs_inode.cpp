#include "tgfs_inode.h"

tgfs_inode::tgfs_inode(struct stat attr, uint64_t version)
    : attr{attr}, version{version} {}

std::vector<uint64_t> tgfs_inode::get_data_version() {
    return std::vector<uint64_t>{data_version_};
}

void tgfs_inode::set_data_version(int n, uint64_t ver) {
    if (n != 0) {
        // Error
    }
    data_version_ = ver;
}
