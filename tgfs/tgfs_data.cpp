#include "tgfs_data.h"

#include <sys/mman.h>

#include <format>

#include "tgfs_helpers.h"

tgfs_data::tgfs_data(bool debug, double timeout, int root_fd,
                     size_t max_filesize, tgfs_net_api *api)
    : api_{api},
      timeout_{timeout},
      root_fd_{root_fd},
      root_path_{std::format("/proc/self/fd/{}/", root_fd)},
      table_path_{std::format("{}message_table", root_path_)},
      max_filesize_{max_filesize},
      debug_{debug},
      inodes_{},
      messages_{table_path_} {
    update_table();
    tgfs_dir *root = make_new_files<tgfs_dir>(*this, FUSE_ROOT_ID);
    new (root) tgfs_dir(root_path_, FUSE_ROOT_ID, FUSE_ROOT_ID);
    inodes_.emplace(FUSE_ROOT_ID, reinterpret_cast<tgfs_inode *>(root));
    upload(FUSE_ROOT_ID);
}

int tgfs_data::update_table() {
    if (api_->is_up_to_date_table()) {
        return 0;
    }
    std::destroy_at(&messages_);
    int err = 0;
    do {
        err = api_->download_table(table_path_);
    } while (err != 0 && err != 2);
    std::construct_at(&messages_, table_path_);
    if (err == 2) {
        messages_.init();
    }
    return 0;
}

tgfs_data *tgfs_data::tgfs_ptr(fuse_req_t req) {
    return reinterpret_cast<tgfs_data *>(fuse_req_userdata(req));
}

bool tgfs_data::is_debug() const {
    return debug_;
}

double tgfs_data::get_timeout() const {
    return timeout_;
}

int tgfs_data::get_root_fd() const {
    return root_fd_;
}

size_t tgfs_data::get_max_filesize() const {
    return max_filesize_;
}

uint64_t tgfs_data::lookup_msg(fuse_ino_t ino) {
    update_table();
    return messages_.at(ino);
}

tgfs_inode *tgfs_data::lookup_inode(fuse_ino_t ino) {
    if (!inodes_.contains(ino)) {
        if (update(ino)) {
            return nullptr;
        }
    }
    return inodes_.at(ino);
}

tgfs_dir *tgfs_data::lookup_dir(fuse_ino_t ino) {
    tgfs_inode *dir = lookup_inode(ino);
    if (dir == nullptr) {
        return nullptr;
    }
    if (!S_ISDIR(dir->attr.st_mode)) {
        // TODO: Send error info
        //       Exception perhaps ?
        //       Or just set errno ?
        return nullptr;
    }
    return reinterpret_cast<tgfs_dir *>(dir);
}

int tgfs_data::upload(fuse_ino_t ino) {
    uint64_t msg = lookup_msg(ino);
    tgfs_inode *ino_obj = lookup_inode(ino);
    ino_obj->upload_data(api_, 0, root_path_);
    uint64_t new_msg = api_->upload(std::format("{}{}/inode", root_path_, ino));
    if (msg != 0) {
        api_->remove(msg);
    }
    messages_.set(ino, new_msg);
    messages_.sync();
    api_->upload_table(table_path_);
    return 0;
}

int tgfs_data::upload(tgfs_inode *ino) {
    inodes_[ino->attr.st_ino] = ino;
    return upload(ino->attr.st_ino);
}

int tgfs_data::update(fuse_ino_t ino) {
    uint64_t msg = lookup_msg(ino);
    if (msg == 0) {
        return 1;
    }
    if (inodes_.contains(ino)) {
        if (msg == inodes_[ino]->version) {
            return 0;
        }
        if (S_ISDIR(inodes_[ino]->attr.st_mode)) {
            munmap(inodes_[ino], sizeof(tgfs_dir));
        } else {
            munmap(inodes_[ino], sizeof(tgfs_inode));
        }
        inodes_.erase(ino);
    }
    api_->download(msg, std::format("{}{}/inode", root_path_, ino));
    inodes_[ino] = map_inode<tgfs_inode>(*this, ino);
    if (S_ISDIR(inodes_[ino]->attr.st_mode)) {
        munmap(inodes_[ino], sizeof(tgfs_dir));
        inodes_[ino] = map_inode<tgfs_dir>(*this, ino);
    }
    inodes_[ino]->update_data(api_, 0, root_path_);
    return 0;
}

fuse_ino_t tgfs_data::new_ino() {
    return ++last_ino_;
}
