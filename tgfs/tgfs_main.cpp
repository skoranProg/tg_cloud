#define FUSE_USE_VERSION 34

#include "tgfs.h"
#include <random>

std::mt19937_64 rnd;

extern struct fuse_lowlevel_ops tgfs_opers;

//

tgfs_data::tgfs_data(double timeout, int root_fd)
    : timeout{timeout}, root_fd{root_fd} {}

double tgfs_data::get_timeout() { return timeout; }

std::unordered_map<fuse_ino_t, int> &tgfs_data::get_fds() { return fds; }

std::unordered_map<fuse_ino_t, uint64_t> &tgfs_data::get_messages() {
  return messages;
}

std::unordered_map<int, tgfs_ftable> &tgfs_data::get_directories() {
  return directories;
}

tgfs_data *tgfs_data::tgfs_ptr(fuse_req_t req) {
  return reinterpret_cast<tgfs_data *>(fuse_req_userdata(req));
}

uint64_t tgfs_data::lookup_msg(fuse_ino_t ino) {
  if (!get_messages().contains(ino)) {
    return 0;
  }
  return get_messages()[ino];
}

tgfs_table &tgfs_data::lookup_dir_ftable(fuse_ino_t ino) {
  if (!get_directories().contains(ino)) {
    return nullptr;
  }
  return get_directories()[ino];
}

int tgfs_upload(fuse_ino_t ino) {
  // TODO
}

int tgfs_update(fuse_ino_t ino) {
  // TODO
}

//

fuse_ino_t get_new_ino(fuse_req_t req) {
  auto context = tgfs_data::tgfs_ptr(req);
  fuse_ino_t res = rnd();
  while (context->get_messages().contains(res) || res == 0) {
    ++res;
  }
  return res;
}

int make_new_tgfs(int argc, char *argv[]) {
  // TODO
}
