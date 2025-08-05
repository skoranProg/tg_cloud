#include "tgfs_data.h"

std::unordered_map<fuse_ino_t, uint64_t> &tgfs_data::get_messages() {
    // TODO: Sync the table
    return messages;
}

tgfs_data::tgfs_data(bool debug, double timeout, int root_fd,
                     size_t max_filesize, TdClass *tdclient)
    : tdclient{tdclient}, debug{debug}, timeout{timeout}, root_fd{root_fd},
      max_filesize{max_filesize}, inodes{}, messages{} {
    tgfs_dir *root = new tgfs_dir(FUSE_ROOT_ID, FUSE_ROOT_ID);
    inodes.emplace(FUSE_ROOT_ID, reinterpret_cast<tgfs_inode *>(root));
}

tgfs_data *tgfs_data::tgfs_ptr(fuse_req_t req) {
    return reinterpret_cast<tgfs_data *>(fuse_req_userdata(req));
}

bool tgfs_data::is_debug() const { return debug; }

double tgfs_data::get_timeout() const { return timeout; }

int tgfs_data::get_root_fd() const { return root_fd; }

size_t tgfs_data::get_max_filesize() const { return max_filesize; }

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
    return inodes.at(ino);
}

tgfs_dir *tgfs_data::lookup_dir(fuse_ino_t ino) {
    tgfs_inode *dir = lookup_inode(ino);
    if (dir == nullptr) {
        return nullptr;
    }
    if (!S_ISDIR(dir->get_attr().st_mode)) {
        // TODO: Send error info
        //       Exception perhaps ?
        //       Or just set errno ?
        return nullptr;
    }
    return reinterpret_cast<tgfs_dir *>(dir);
}

int tgfs_data::upload(fuse_ino_t ino) {
    // TODO
    return 0;
}

int tgfs_data::upload(tgfs_inode *ino) {
    inodes[ino->get_attr().st_ino] = ino;
    return upload(ino->get_attr().st_ino);
}

int tgfs_data::update(fuse_ino_t ino) {
    // TODO
    return 0;
}
