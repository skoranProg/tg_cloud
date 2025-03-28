#define FUSE_USE_VERSION 34

#include "errno.h"
#include "tgfs.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

void tgfs_lookup(fuse_req_t req, fuse_ino_t parent, const char *name) {
  auto context = tgfs_data::tgfs_ptr(req);
  if (!context->lookup_dir_ftable(parent).contains(name)) {
    fuse_reply_err(req, ENOENT);
    return;
  }
  struct fuse_entry_param e = {
      .ino = context->lookup_dir_ftable(parent)[name],
      .attr_timeout = context->get_timeout(),
      .entry_timeout = context->get_timeout(),
  };
  if (fstatat(context->get_root_fd(), std::to_string(e.ino).c_str(), &e.attr,
              AT_SYMLINK_NOFOLLOW) == -1) {
    fuse_reply_err(req, errno);
    return;
  }
  e.attr.st_ino = e.ino;
  fuse_reply_entry(req, &e);
}

void tgfs_mknod(fuse_req_t req, fuse_ino_t parent, const char *name,
                mode_t mode, dev_t rdev) {
  if (!S_ISREG(mode)) {
    fuse_reply_err(req, ENOSYS);
    return;
  }
  auto context = tgfs_data::tgfs_ptr(req);
  if (context->lookup_dir_ftable(parent).contains(name)) {
    fuse_reply_err(req, EEXIST);
    return;
  }
  fuse_ino_t nod_ino = get_new_ino(req);
  if (mknodat(context->get_root_fd(), std::to_string(nod_ino).c_str(), mode,
              rdev) == -1) {
    fuse_reply_err(req, errno);
    return;
  }
  if (context->tgfs_upload(nod_ino) != 0) {
    // TODO : handle exception
  }
  context->lookup_dir_ftable(parent)[name] = nod_ino;
  if (context->tgfs_upload(parent) != 0) {
    // TODO : handle exception
  }
}

void tgfs_readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
                  struct fuse_file_info *fi) {
  // TODO
}

void tgfs_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
  // TODO
}

void tgfs_read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
               struct fuse_file_info *fi) {
  // TODO
}

void tgfs_write(fuse_req_t req, fuse_ino_t ino, const char *buf, size_t size,
                off_t off, struct fuse_file_info *fi) {
  // TODO
}

void tgfs_flush(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
  // TODO
}

void tgfs_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
  // TODO
}

struct fuse_lowlevel_ops tgfs_opers = {
    .lookup = tgfs_lookup,
    .getattr = tgfs_getattr,
    .mknod = tgfs_mknod,
    .open = tgfs_open,
    .read = tgfs_read,
    .write = tgfs_write,
    .flush = tgfs_flush,
    .readdir = tgfs_readdir,
};
