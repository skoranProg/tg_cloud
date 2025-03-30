#ifndef _TGFS_DATA_H_
#define _TGFS_DATA_H_

#include "tgfs_fuse_dependencies.h"

#include "../tdclient.h"
#include "tgfs_dir.h"
#include "tgfs_inode.h"

class tgfs_data {
private:
  TdClass *tdclient;
  double timeout;
  const int root_fd;
  const size_t max_filesize;
  bool debug;
  // Only physically present(downloaded) files
  std::unordered_map<fuse_ino_t, tgfs_inode> inodes; // ino -> inode
  // All files on server
  std::unordered_map<fuse_ino_t, uint64_t> messages; // ino -> msg_id

  std::unordered_map<fuse_ino_t, uint64_t> &get_messages();

public:
  tgfs_data(bool debug, double timeout, int root_fd, size_t max_filesize,
            TdClass *tdclient);

  static tgfs_data *tgfs_ptr(fuse_req_t req);

  bool is_debug();

  double get_timeout();

  int get_root_fd();

  size_t get_max_filesize();

  uint64_t lookup_msg(fuse_ino_t ino);

  tgfs_inode *lookup_inode(fuse_ino_t ino);

  tgfs_dir *lookup_dir(fuse_ino_t ino);

  int upload(fuse_ino_t ino);
  int update(fuse_ino_t ino);
};

#endif