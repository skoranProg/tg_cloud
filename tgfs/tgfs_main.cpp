#include "tgfs.h"
#include "tgfs_data.h"

#include <random>

std::mt19937_64 rnd;

extern struct fuse_lowlevel_ops tgfs_opers;

fuse_ino_t get_new_ino(fuse_req_t req) {
  auto context = tgfs_data::tgfs_ptr(req);
  fuse_ino_t res = rnd();
  while (context->get_messages().contains(res) || res == 0) {
    ++res;
  }
  return res;
}

int make_new_tgfs(int argc, char *argv[], TdClass tdclient) {
  // TODO
}
