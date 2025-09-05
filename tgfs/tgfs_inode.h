#ifndef _TGFS_INODE_H_
#define _TGFS_INODE_H_

#include <sys/stat.h>

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "tgfs.h"
#include "tgfs_fuse_dependencies.h"

class tgfs_inode {
 public:
    struct stat *attr;
    uint64_t version;

    int update_data(tgfs_net_api *api, int n, const std::string &root_path);
    int upload_data(tgfs_net_api *api, int n, const std::string &root_path);

    tgfs_inode(fuse_ino_t ino, const std::string &root_path);
    ~tgfs_inode();

 private:
    uint64_t data_msg_;
    uint64_t data_version_;
};

#endif
