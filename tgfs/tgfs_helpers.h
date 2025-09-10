#ifndef _TGFS_HELPERS_
#define _TGFS_HELPERS_

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <format>
#include <string>

#include "tgfs_data.h"

template <class T>
concept DerivedFromInode = std::is_base_of<tgfs_inode, T>::value;

template <DerivedFromInode T>
T *make_new_files(const tgfs_data &context, fuse_ino_t ino) {
    if (mkdir(std::format("{}/{}", context.get_root_path(), ino).c_str(),
              S_IFDIR | 0755) == -1 &&
        errno != EEXIST) {
        return nullptr;
    }

    int fd =
        creat(std::format("{}/{}/data_0", context.get_root_path(), ino).c_str(),
              0666);
    if (fd == -1) {
        return nullptr;
    }
    close(fd);

    return new T(ino, context.get_root_path());
}

#endif
