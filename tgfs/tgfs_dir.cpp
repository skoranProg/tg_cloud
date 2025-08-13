#include "tgfs_dir.h"

tgfs_dir::tgfs_dir(fuse_ino_t self, fuse_ino_t parent)
    : tgfs_inode{(struct stat){.st_dev = 0,
                               .st_ino = self,
                               .st_nlink = 1,
                               .st_mode = S_IFDIR | S_IRWXU,
                               .st_uid = 0,
                               .st_gid = 0,
                               .st_rdev = 0,
                               .st_size = 0,
                               .st_blksize = 0,
                               .st_blocks = 1,
                               .st_atim = {},
                               .st_mtim = {},
                               .st_ctim = {}},
                 (uint64_t)0},
      ftable{}, rev_ftable{} {
    ftable.emplace("..", parent);
    rev_ftable.emplace(parent, "..");
}

bool tgfs_dir::contains(const std::string &name) {
    return ftable.contains(name);
}

bool tgfs_dir::contains(fuse_ino_t ino) {
    return (*rev_ftable.lower_bound(std::make_pair(ino, ""))).first == ino;
}

int tgfs_dir::add(const std::string &name, fuse_ino_t ino) {
    if (contains(name)) {
        return -1;
    }
    ftable.emplace(name, ino);
    rev_ftable.emplace(ino, name);
    return 0;
}

fuse_ino_t tgfs_dir::lookup(const std::string &name) {
    if (!contains(name)) {
        return 0;
    }
    return ftable.at(name);
}

const std::pair<fuse_ino_t, std::string> *tgfs_dir::next(fuse_ino_t ino) const {
    if (ino + 1 <= 0) {
        return nullptr;
    }
    auto ub_iter = rev_ftable.lower_bound(std::make_pair(ino + 1, ""));
    if (ub_iter == rev_ftable.end()) {
        return nullptr;
    }
    return &(*ub_iter);
}
