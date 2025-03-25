#ifndef TGFS_H
#define TGFS_H

#include <fuse_lowlevel.h>
#include <string>
#include <unordered_map>

class tgfs_inode {
public:
  int fd;
  fuse_ino_t ino;

public:
  static tgfs_inode *inode_ptr(fuse_req_t req, fuse_ino_t ino);
};

using tgfs_ftable = std::unordered_map<std::string, ino_t>;

class tgfs_data {
private:
  double timeout;
  const int root_fd;
  std::unordered_map<fuse_ino_t, tgfs_inode *> inodes; // ino -> inode*
  std::unordered_map<fuse_ino_t, uint64_t> messages;   // ino -> msg_id
  std::unordered_map<int, tgfs_ftable> directories;    // fd -> ftable

public:
  tgfs_data(double timeout, int root_fd);

  static tgfs_data *tgfs_ptr(fuse_req_t req);

  double get_timeout();

  int get_root_fd();

  std::unordered_map<fuse_ino_t, tgfs_inode *> &get_inodes();

  std::unordered_map<fuse_ino_t, uint64_t> &get_messages();

  std::unordered_map<int, tgfs_ftable> &get_directories();

  tgfs_inode *lookup_inode(fuse_ino_t ino);

  uint64_t lookup_msg(fuse_ino_t ino);

  tgfs_table &lookup_dir_ftable(in fd);
};

int make_new_tgfs(int argc, char *argv[], std::string mountpoint);

fuse_ino_t get_new_ino(fuse_req_t req);
#endif
