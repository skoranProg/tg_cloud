#include "tgfs_inode.h"

tgfs_inode::tgfs_inode(struct stat attr, uint64_t version)
    : attr_{attr}, version_{version} {}

struct stat tgfs_inode::get_attr() { return attr_; }

void tgfs_inode::set_attr(struct stat attr) { attr_ = attr; }

uint64_t tgfs_inode::get_version() { return version_; }

void tgfs_inode::set_version(uint64_t version) { version_ = version; }
