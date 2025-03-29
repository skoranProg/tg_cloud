#include "tgfs_dir.h"

tgfs_dir::tgfs_dir(fuse_ino_t self, fuse_ino_t parent)
    : ino{self}, ftable{}, rev_ftable{} {
  ftable.emplace(".", self);
  rev_ftable.emplace(self, ".");
  ftable.emplace("..", parent);
  rev_ftable.emplace(parent, "..");
}

bool tgfs_dir::contains(const std::string &name) {
  return ftable.contains(name);
}

bool tgfs_dir::contains(fuse_ino_t ino) {
  return rev_ftable.contains(ino);
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
  return ftable[name];
}
