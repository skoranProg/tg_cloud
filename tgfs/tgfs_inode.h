#ifndef _TGFS_INODE_H_
#define _TGFS_INODE_H_

#include <sys/stat.h>

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "tgfs_fuse_dependencies.h"

class tgfs_inode {
 public:
    struct stat attr;
    uint64_t version;

    std::vector<uint64_t> get_data_version();
    void set_data_version(int n, uint64_t ver);

    tgfs_inode(struct stat attr, uint64_t version);

 private:
    uint64_t data_version_;
};

#endif
