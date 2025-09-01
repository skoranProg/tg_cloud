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
T *map_inode(const tgfs_data &context, fuse_ino_t ino) {
    int fd =
        open(std::format("{}{}/inode", context.get_root_path(), ino).c_str(),
             O_RDWR);
    T *res = reinterpret_cast<T *>(
        mmap(NULL, sizeof(T), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    close(fd);
    return res;
}

template <DerivedFromInode T>
T *make_new_files(const tgfs_data &context, fuse_ino_t ino) {
    std::string local_fname = std::to_string(ino);
    if (mkdir((context.get_root_path() + local_fname).c_str(),
              S_IFDIR | 0755) == -1) {
        return nullptr;
    }

    int fd = open(
        std::format("{}{}/inode", context.get_root_path(), local_fname).c_str(),
        O_CREAT | O_RDWR, S_IFREG | 0666);
    ftruncate(fd, sizeof(T));
    T *ino_obj = reinterpret_cast<T *>(
        mmap(NULL, sizeof(T), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    close(fd);

    mknod(std::format("{}{}/data_0", context.get_root_path(), local_fname)
              .c_str(),
          S_IFREG | 0666, 0);
    return ino_obj;
}

#endif
