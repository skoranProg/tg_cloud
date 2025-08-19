#ifndef _TGFS_HELPERS_
#define _TGFS_HELPERS_

#include "tgfs_data.h"
#include <unistd.h>

fuse_ino_t get_new_ino(tgfs_data &context);

tgfs_inode *map_inode(const tgfs_data &context, fuse_ino_t ino);

#endif
