#define FUSE_USE_VERSION 34

#include "tgfs.h"
#include <random>

std::mt19937_64 rnd;

extern struct fuse_lowlevel_ops tgfs_opers;

//

tgfs_inode *tgfs_inode::inode_ptr(fuse_req_t req, fuse_ino_t ino) {
  return tgfs_data::tgfs_ptr(req)->get_inodes()[ino];
}

//

tgfs_data::tgfs_data(double timeout, int root_fd)
    : timeout{timeout}, root_fd{root_fd} {}

double tgfs_data::get_timeout() { return timeout; }

std::unordered_map<fuse_ino_t, tgfs_inode *> &tgfs_data::get_inodes() {
  return inodes;
}

std::unordered_map<fuse_ino_t, uint64_t> &tgfs_data::get_messages() {
  return messages;
}

std::unordered_map<int, tgfs_ftable> &tgfs_data::get_directories() {
  return directories;
}

tgfs_data *tgfs_data::tgfs_ptr(fuse_req_t req) {
  return reinterpret_cast<tgfs_data *>(fuse_req_userdata(req));
}

tgfs_inode *tgfs_data::lookup_inode(fuse_ino_t ino) {
  if (!get_inodes().contains(ino)) {
    return nullptr;
  }
  return get_inodes()[ino];
}

uint64_t lookup_msg(fuse_ino_t ino) {
  if (!get_messages().contains(ino)) {
    return 0;
  }
  return get_messages()[ino];
}

tgfs_table &lookup_dir_ftable(in fd) {
  if(!get_directories().contains(fd)) {
    return nullptr;
  }
  return get_directories()[fd];
}

//

fuse_ino_t get_new_ino(fuse_req_t req) {
  auto context = tgfs_data::tgfs_ptr(req);
  fuse_ino_t res = rnd();
  while (context->get_inodes().contains(res) || res == 0) {
    ++res;
  }
  return res;
}

int make_new_tgfs(int argc, char *argv[], std::string mountpoint) {
  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
  struct fuse_session *se;
  struct fuse_cmdline_opts opts;
  struct fuse_loop_config config;
  int ret = -1;

  if (fuse_parse_cmdline(&args, &opts) != 0) {
    return 1;
  }

  se = fuse_session_new(&args, &tgfs_opers, sizeof(tgfs_opers), NULL);
  if (se == NULL)
    goto err_out1;

  if (fuse_set_signal_handlers(se) != 0)
    goto err_out2;

  if (fuse_session_mount(se, mountpoint.c_str()) != 0)
    goto err_out3;

  fuse_daemonize(opts.foreground);

  /* Block until ctrl+c or fusermount -u */
  config.clone_fd = opts.clone_fd;
  config.max_idle_threads = opts.max_idle_threads;
  ret = fuse_session_loop_mt(se, &config);

  fuse_session_unmount(se);
err_out3:
  fuse_remove_signal_handlers(se);
err_out2:
  fuse_session_destroy(se);
err_out1:
  fuse_opt_free_args(&args);

  return ret ? 1 : 0;
}
