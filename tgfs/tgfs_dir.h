#ifndef _TGFS_DIR_H_
#define _TGFS_DIR_H_

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>

#include "tgfs_fuse_dependencies.h"
#include "tgfs_inode.h"
#include "tgfs_table.h"

class tgfs_dir : public tgfs_inode, public tgfs_table<std::string, fuse_ino_t> {
 public:
    tgfs_dir(fuse_ino_t ino, const std::string &root_path);

    int init(fuse_ino_t parent_dir);

    virtual int upload_data(tgfs_net_api *api, int n,
                            const std::string &root_path) override;
    virtual int update_data(tgfs_net_api *api, int n,
                            const std::string &root_path) override;

    std::vector<std::tuple<uint64_t, std::string, fuse_ino_t>> next(
        uint64_t off, int n) const;

    bool empty();
};

#endif
