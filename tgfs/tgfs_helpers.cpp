#include "tgfs_helpers.h"

#include <fcntl.h>
#include <sys/mman.h>

#include <format>
#include <random>
#include <string>

std::mt19937_64 rnd;

fuse_ino_t get_new_ino(tgfs_data &context) {
    fuse_ino_t res = static_cast<fuse_ino_t>(rnd());
    while (res == 0 || context.lookup_msg(res) != 0) {
        ++res;
    }
    return res;
}

tgfs_inode *map_inode(const tgfs_data &context, fuse_ino_t ino) {
    int fd = openat(context.get_root_fd(), std::format("{}/inode", ino).c_str(),
                    O_RDWR);
    tgfs_inode *res = reinterpret_cast<tgfs_inode *>(mmap(
        NULL, sizeof(tgfs_inode), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    close(fd);
    return res;
}
