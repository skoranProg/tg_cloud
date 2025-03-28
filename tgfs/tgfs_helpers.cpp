#include "tgfs_helpers.h"
#include "tgfs_data.h"

#include <random>

std::mt19937_64 rnd;

fuse_ino_t get_new_ino(fuse_req_t req) {
  tgfs_data *context = tgfs_data::tgfs_ptr(req);
  fuse_ino_t res = rnd();
  while (context->get_messages().contains(res) || res == 0) {
    ++res;
  }
  return res;
}
