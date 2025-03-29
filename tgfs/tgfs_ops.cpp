#include "tgfs_data.h"
#include "tgfs_helpers.h"

#include "errno.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

void tgfs_lookup(fuse_req_t req, fuse_ino_t parent, const char *name) {
  tgfs_data *context = tgfs_data::tgfs_ptr(req);
  tgfs_dir *parent_dir = context->lookup_dir(parent);
  if (!parent_dir->contains(name)) {
    fuse_reply_err(req, ENOENT);
    return;
  }
  struct fuse_entry_param e = {
      .ino = parent_dir->lookup(name),
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
  tgfs_data *context = tgfs_data::tgfs_ptr(req);
  tgfs_dir *parent_dir = context->lookup_dir(parent);
  if (parent_dir->contains(name)) {
    fuse_reply_err(req, EEXIST);
    return;
  }
  fuse_ino_t nod_ino = get_new_ino(req);
  if (mknodat(context->get_root_fd(), std::to_string(nod_ino).c_str(), mode,
              rdev) == -1) {
    fuse_reply_err(req, errno);
    return;
  }
  if (context->upload(nod_ino) != 0) {
    // TODO : handle exception
  }
  parent_dir->add(name, nod_ino);
  if (context->upload(parent) != 0) {
    // TODO : handle exception
  }
}

void tgfs_readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
                  struct fuse_file_info *fi) {
  tgfs_data *context = tgfs_data::tgfs_ptr(req);
  const tgfs_dir *dir = context->lookup_dir(ino);
  char buf[size];
  char *p = buf;
  size_t rem = size;
  off_t nextoff = off;
  int err = 0;

  while (true) {
    size_t entsize;
    const char *name;
    const std::pair<fuse_ino_t, std::string> *ent;
    ent = dir->next(off);
    if (ent == nullptr) {
      break;
    }
    nextoff = ent->first;
    struct stat st;
    if (fstatat(context->get_root_fd(), std::to_string(ent->first).c_str(), &st,
                AT_SYMLINK_NOFOLLOW) == -1) {
      err = errno;
      break;
    }
    st.st_ino = ent->first;
    entsize = fuse_add_direntry(req, p, rem, ent->second.c_str(), &st, nextoff);
    if (entsize > rem) {
      break;
    }
    p += entsize;
    rem -= entsize;
  }

  if (err != 0 && rem == size) {
    fuse_reply_err(req, err);
    return;
  }
  fuse_reply_buf(req, buf, size - rem);
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
  tgfs_data *context = tgfs_data::tgfs_ptr(req);
  struct stat st;
  if (fstatat(context->get_root_fd(), std::to_string(ino).c_str(), &st,
              AT_SYMLINK_NOFOLLOW) == -1) {
    fuse_reply_err(req, errno);
    return;
  }
  st.st_ino = ino;
  fuse_reply_attr(req, &st, context->get_timeout());
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
