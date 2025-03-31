#include "tgfs_data.h"
#include "../tgcl/td_helpers.h"

std::unordered_map<fuse_ino_t, uint64_t> &tgfs_data::get_messages() {
  return messages;
}

std::unordered_map<fuse_ino_t, tgfs_dir> &tgfs_data::get_directories() {
  return directories;
}

int tgfs_data::upload_table() {
  TableTdUpdater tu(tdclient);
  tu.upload_table(messages);
  return 0;
}

int tgfs_data::update_table() {
  TableTdUpdater tu(tdclient);
  messages = tu.get_table();
  return 0;

}

tgfs_data::tgfs_data(bool debug, double timeout, int root_fd,
                     size_t max_filesize, TdClass *tdclient)
    : tdclient{tdclient}, debug{debug}, timeout{timeout}, root_fd{root_fd},
      max_filesize{max_filesize}, last_version{}, messages{}, directories{} {
  directories.try_emplace(FUSE_ROOT_ID, FUSE_ROOT_ID, FUSE_ROOT_ID);
}

tgfs_data *tgfs_data::tgfs_ptr(fuse_req_t req) {
  return reinterpret_cast<tgfs_data *>(fuse_req_userdata(req));
}

bool tgfs_data::is_debug() {
  return debug;
}

double tgfs_data::get_timeout() { return timeout; }

int tgfs_data::get_root_fd() { return root_fd; }

size_t tgfs_data::get_max_filesize() { return max_filesize; }

uint64_t tgfs_data::lookup_msg(fuse_ino_t ino) {
  if (!get_messages().contains(ino)) {
    return 0;
  }
  return get_messages().at(ino);
}

tgfs_dir *tgfs_data::lookup_dir(fuse_ino_t ino) {
  if (!get_directories().contains(ino)) {
    return nullptr;
  }
  return &get_directories().at(ino);
}

int tgfs_data::upload(fuse_ino_t ino) {
  // TODO
  return 0;
}

int tgfs_data::update(fuse_ino_t ino) {
  // TODO
  return 0;
}