#ifndef _TGFS_DATA_H_
#define _TGFS_DATA_H_

#include "tgfs_fuse_dependencies.h"

#include "../tgcl/tdclient.h"
#include "tgfs_dir.h"

class tgfs_data {
private:
  TdClass *tdclient;
  double timeout;
  const int root_fd;
  const size_t max_filesize;
  bool debug;
  // Only physically present(downloaded) files
  std::unordered_map<fuse_ino_t, uint64_t> last_version; // ino -> msg_id
  // All files on server
  std::unordered_map<fuse_ino_t, uint64_t> messages; // ino -> msg_id
  // Physically present directories(their content might not be downloaded)
  std::unordered_map<fuse_ino_t, tgfs_dir> directories; // ino -> ftable

  std::unordered_map<fuse_ino_t, uint64_t> &get_messages();

  std::unordered_map<fuse_ino_t, tgfs_dir> &get_directories();

  int upload_table();

  int update_table();

public:
  tgfs_data(bool debug, double timeout, int root_fd, size_t max_filesize,
            TdClass *tdclient);

  static tgfs_data *tgfs_ptr(fuse_req_t req);

  bool is_debug();

  double get_timeout();

  int get_root_fd();

  size_t get_max_filesize();

  uint64_t lookup_msg(fuse_ino_t ino);

  tgfs_dir *lookup_dir(fuse_ino_t ino);

  int upload(fuse_ino_t ino);
  int update(fuse_ino_t ino);
};

#endif