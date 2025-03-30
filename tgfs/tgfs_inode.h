#ifndef _TGFS_INODE_H_
#define _TGFS_INODE_H_

#include "sys/stat.h"
#include <cstdint>
#include <map>
#include <set>
#include <string>

#include "tgfs_fuse_dependencies.h"

class tgfs_inode {
public:
  struct stat attr;
  uint64_t version;
};

#endif