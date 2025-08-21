#ifndef _TGFS_INODE_H_
#define _TGFS_INODE_H_

#include "tgfs_fuse_dependencies.h"
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <sys/stat.h>

class tgfs_inode {
  public:
    struct stat get_attr();
    void set_attr(struct stat attr);

    uint64_t get_version();
    void set_version(uint64_t version);

    tgfs_inode(struct stat attr, uint64_t version);

  private:
    struct stat attr_;
    uint64_t version_;
};

#endif
