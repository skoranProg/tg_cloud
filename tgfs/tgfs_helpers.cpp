#include "tgfs_helpers.h"

#include <random>

std::mt19937_64 rnd;

fuse_ino_t get_new_ino(tgfs_data &context) {
    fuse_ino_t res = static_cast<fuse_ino_t>(rnd());
    while (res == 0 || context.lookup_msg(res) != 0) {
        ++res;
    }
    return res;
}
