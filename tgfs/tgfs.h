#ifndef TGFS_H
#define TGFS_H

#include "fuse_lowlevel.h"
#include "tdclient.h"
#include <string>
#include <unordered_map>

using tgfs_ftable = std::unordered_map<std::string, ino_t>;

class tgfs_data {
private:
  TdClass tdclient;
  double timeout;
  const int root_fd;
  // Only physically present(downloaded) files
  std::unordered_map<fuse_ino_t, uint64_t> last_version; // ino -> msg_id
  // All files on server
  std::unordered_map<fuse_ino_t, uint64_t> messages; // ino -> msg_id
  // Physically present directories(their content might not be downloaded)
  std::unordered_map<fuse_ino_t, tgfs_ftable> directories; // ino -> ftable

public:
  tgfs_data(double timeout, int root_fd);

  static tgfs_data *tgfs_ptr(fuse_req_t req);

  double get_timeout();

  int get_root_fd();

  std::unordered_map<fuse_ino_t, int> &get_fds();

  std::unordered_map<fuse_ino_t, uint64_t> &get_messages();

  std::unordered_map<int, tgfs_ftable> &get_directories();

  uint64_t lookup_msg(fuse_ino_t ino);

  tgfs_ftable &lookup_dir_ftable(fuse_ino_t ino);

  int upload(fuse_ino_t ino);
  int update(fuse_ino_t ino);
};

int make_new_tgfs(int argc, char *argv[], std::string mountpoint);

fuse_ino_t get_new_ino(fuse_req_t req);
#endif
