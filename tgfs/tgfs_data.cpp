#include "tgfs_data.h"

std::unordered_map<fuse_ino_t, uint64_t> &tgfs_data::get_messages() {
  return messages;
}

std::unordered_map<fuse_ino_t, tgfs_dir> &tgfs_data::get_directories() {
  return directories;
}

tgfs_data::tgfs_data(double timeout, int root_fd, TdClass *tdclient)
    : timeout{timeout}, root_fd{root_fd}, tdclient{tdclient} {}

tgfs_data *tgfs_data::tgfs_ptr(fuse_req_t req) {
  return reinterpret_cast<tgfs_data *>(fuse_req_userdata(req));
}

double tgfs_data::get_timeout() { return timeout; }

int tgfs_data::get_root_fd() { return root_fd; }

uint64_t tgfs_data::lookup_msg(fuse_ino_t ino) {
  if (!get_messages().contains(ino)) {
    return 0;
  }
  return get_messages()[ino];
}

tgfs_dir *tgfs_data::lookup_dir(fuse_ino_t ino) {
  if (!get_directories().contains(ino)) {
    return nullptr;
  }
  return &get_directories()[ino];
}

int tgfs_upload(fuse_ino_t ino) {
  // TODO
  return 0;
}

int tgfs_update(fuse_ino_t ino) {
  // TODO
  return 0;
}