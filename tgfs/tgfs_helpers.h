#ifndef _TGFS_HELPERS_
#define _TGFS_HELPERS_

#include <unistd.h>

#include "tgfs_data.h"

tgfs_inode *map_inode(const tgfs_data &context, fuse_ino_t ino);

#endif
