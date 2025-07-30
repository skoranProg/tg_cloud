#include "tgfs_data.h"

std::unordered_map<fuse_ino_t, uint64_t> &tgfs_data::get_messages() {
    return messages;
}

std::unordered_map<fuse_ino_t, tgfs_dir> &tgfs_data::get_directories() {
    return directories;
}

tgfs_data::tgfs_data(bool debug, double timeout, int root_fd,
                     size_t max_filesize, TdClass *tdclient)
    : tdclient{tdclient}, debug{debug}, timeout{timeout}, root_fd{root_fd},
      max_filesize{max_filesize}, last_version{}, messages{}, directories{} {
    tgfs_dir root(FUSE_ROOT_ID, FUSE_ROOT_ID);
    root.version = 0;
    fstatat(root_fd, "", &root.attr, AT_EMPTY_PATH);
    root.attr.st_ino = FUSE_ROOT_ID;
    inodes.insert(FUSE_ROOT_ID, std::move(root));
}

tgfs_data *tgfs_data::tgfs_ptr(fuse_req_t req) {
    return reinterpret_cast<tgfs_data *>(fuse_req_userdata(req));
}

bool tgfs_data::is_debug() { return debug; }

double tgfs_data::get_timeout() { return timeout; }

int tgfs_data::get_root_fd() { return root_fd; }

size_t tgfs_data::get_max_filesize() { return max_filesize; }

uint64_t tgfs_data::lookup_msg(fuse_ino_t ino) {
    if (!get_messages().contains(ino)) {
        return 0;
    }
    return get_messages().at(ino);
}

tgfs_inode *tgfs_data::lookup_inode(fuse_ino_t ino) {
    if (!get_messages().contains(ino)) {
        return nullptr;
    }
    return &inodes.at(ino);
}

tgfs_dir *tgfs_data::lookup_dir(fuse_ino_t ino) {
    tgfs_inode *dir = lookup_inode(ino);
    if (dir == nullptr || !S_ISDIR(dir->attr.st_mode)) {
        return nullptr;
    }
    return reinterpret_cast<tgfs_dir *>(dir);
}

int tgfs_data::upload(fuse_ino_t ino) {
    // TODO
    return 0;
}

int tgfs_data::update(fuse_ino_t ino) {
    // TODO
    return 0;
}
