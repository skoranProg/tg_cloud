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
    tgfs_dir(const std::string &root_path, fuse_ino_t self, fuse_ino_t parent);

    std::vector<std::tuple<uint64_t, std::string, fuse_ino_t>> next(fuse_ino_t ino, int n) const;
};

#endif
