#ifndef _TGFS_DATA_H_
#define _TGFS_DATA_H_

#include "tgfs_fuse_dependencies.h"

#include "../tdclient.h"

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
  std::unordered_map<fuse_ino_t, tgfs_dir> directories; // ino -> ftable

  std::unordered_map<fuse_ino_t, uint64_t> &get_messages();

  std::unordered_map<fuse_ino_t, tgfs_dir> &get_directories();

public:
  tgfs_data(double timeout, int root_fd);

  static tgfs_data *tgfs_ptr(fuse_req_t req);

  double get_timeout();

  int get_root_fd();

  uint64_t lookup_msg(fuse_ino_t ino);

  tgfs_dir *lookup_dir(fuse_ino_t ino);

  int upload(fuse_ino_t ino);
  int update(fuse_ino_t ino);
};

#endif