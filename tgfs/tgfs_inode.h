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
    uint64_t nlookup;
    uint64_t version;

    virtual int update_data(tgfs_net_api *api, int n,
                            const std::string &root_path);
    virtual int upload_data(tgfs_net_api *api, int n,
                            const std::string &root_path);
    void remove_data(tgfs_net_api *api);

    tgfs_inode(fuse_ino_t ino, const std::string &root_path);
    virtual ~tgfs_inode();

 private:
    uint64_t data_msg_;
    uint64_t data_version_;
};

#endif
