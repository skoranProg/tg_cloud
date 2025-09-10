#include "tgfs_data.h"

#include <stdio.h>
#include <sys/mman.h>

#include <format>
#include <iostream>

#include "tgfs_helpers.h"

tgfs_data::tgfs_data(bool debug, double timeout, int root_fd,
                     size_t max_filesize, tgfs_net_api *api,
                     const std::string &root_path)
    : api_{api},
      timeout_{timeout},
      root_fd_{root_fd},
      root_path_{root_path},
      table_path_{std::format("{}/message_table", root_path_)},
      max_filesize_{max_filesize},
      debug_{debug},
      last_ino_{0},
      inodes_{},
      messages_{table_path_} {
    update_table(true);
    if (lookup_msg(FUSE_ROOT_ID) != 0) {
        last_ino_ = messages_.max_key();
        return;
    }
    ::remove(std::format("{}/{}/inode", root_path_, FUSE_ROOT_ID).c_str());
    ::remove(std::format("{}/{}/data_0", root_path_, FUSE_ROOT_ID).c_str());
    ::remove(std::format("{}/{}", root_path_, FUSE_ROOT_ID).c_str());
    tgfs_dir *root = make_new_files<tgfs_dir>(*this, FUSE_ROOT_ID);

    struct stat attr = {.st_dev = 0,
                        .st_ino = FUSE_ROOT_ID,
                        .st_nlink = 1,
                        .st_mode = S_IFDIR | S_IRWXU,
                        .st_uid = geteuid(),
                        .st_gid = getegid(),
                        .st_rdev = 0,
                        .st_size = 666,
                        .st_blksize = 0,
                        .st_blocks = 1,
                        .st_atim = {},
                        .st_mtim = {},
                        .st_ctim = {}};
    clock_gettime(CLOCK_REALTIME, &(attr.st_atim));
    attr.st_mtim = attr.st_atim;
    attr.st_ctim = attr.st_atim;

    *reinterpret_cast<struct stat *>(root->attr) = attr;
    root->nlookup = 1;
    root->init(FUSE_ROOT_ID);
    last_ino_ = FUSE_ROOT_ID;
    inodes_.emplace(FUSE_ROOT_ID, reinterpret_cast<tgfs_inode *>(root));
    upload(FUSE_ROOT_ID);
}

int tgfs_data::update_table(bool force = false) {
    std::clog << "tgfs_data::update_table()\n\tforce: "
              << (force ? "true" : "false") << std::endl;
    if (!force && api_->is_up_to_date_table()) {
        return 0;
    }
    std::destroy_at(&messages_);
    int err = 0;
    do {
        err = api_->download_table(table_path_);
    } while (err != 0 && err != 2);
    if (err == 2) {
        ::remove(table_path_.c_str());
    }
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

const std::string &tgfs_data::get_root_path() const {
    return root_path_;
}

size_t tgfs_data::get_max_filesize() const {
    return max_filesize_;
}

uint64_t tgfs_data::lookup_msg(fuse_ino_t ino) {
    std::clog << "tgfs_data::lookup_msg\n\tino: " << ino << std::endl;
    update_table();
    return messages_.at(ino);
}

tgfs_inode *tgfs_data::lookup_inode(fuse_ino_t ino) {
    std::clog << "tgfs_data::lookup_inode\n\tino: " << ino << std::endl;
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
    if (!S_ISDIR(dir->attr->st_mode)) {
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
    ino_obj->metasync();
    uint64_t new_msg =
        api_->upload(std::format("{}/{}/inode", root_path_, ino));
    ino_obj->version = new_msg;
    if (msg != 0) {
        api_->remove(msg);
    }
    messages_.set(ino, new_msg);
    messages_.sync();
    api_->upload_table(table_path_);
    return 0;
}

int tgfs_data::upload(tgfs_inode *ino) {
    inodes_[ino->attr->st_ino] = ino;
    return upload(ino->attr->st_ino);
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
        delete inodes_[ino];
        inodes_.erase(ino);
    }
    mkdir(std::format("{}/{}", root_path_, ino).c_str(), 0755);
    api_->download(msg, std::format("{}/{}/inode", root_path_, ino));
    tgfs_inode *ino_obj = new tgfs_inode(ino, root_path_);
    if (S_ISDIR(ino_obj->attr->st_mode)) {
        delete ino_obj;
        ino_obj = new tgfs_dir(ino, root_path_);
    }
    ino_obj->version = msg;
    inodes_[ino] = ino_obj;
    inodes_[ino]->update_data(api_, 0, root_path_);
    return 0;
}

int tgfs_data::remove(tgfs_inode *ino_obj) {
    fuse_ino_t ino = ino_obj->attr->st_ino;
    inodes_.erase(ino);
    ino_obj->remove_data(api_);
    delete ino_obj;
    uint64_t msg = messages_.at(ino);
    messages_.remove(ino);
    api_->upload_table(table_path_);
    api_->remove(msg);
    ::remove(std::format("{}/{}/inode", root_path_, ino).c_str());
    ::remove(std::format("{}/{}/data_0", root_path_, ino).c_str());
    ::remove(std::format("{}/{}", root_path_, ino).c_str());
    return 0;
}

fuse_ino_t tgfs_data::new_ino() {
    while (messages_.contains(++last_ino_));
    return last_ino_;
}
