#ifndef _TGFS_DIR_H_
#define _TGFS_DIR_H_

#include "tgfs_fuse_dependencies.h"

#include <map>
#include <string>
#include <unordered_map>

class tgfs_dir {
private:
  fuse_ino_t ino;
  std::unordered_map<std::string, fuse_ino_t> ftable;
  std::map<fuse_ino_t, std::string> rev_ftable;

public:
  tgfs_dir(fuse_ino_t self, fuse_ino_t parent);
  bool contains(std::string name);
  bool contains(fuse_ino_t ino);
  int add(std::string name, fuse_ino_t ino);
  fuse_ino_t lookup(std::string name);
};
#endif