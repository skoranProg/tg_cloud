#include "tgfs_data.h"
#include "tgfs_helpers.h"

#include "errno.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

void tgfs_lookup(fuse_req_t req, fuse_ino_t parent, const char *name) {
  tgfs_data *context = tgfs_data::tgfs_ptr(req);

  if (context->is_debug()) {
    fuse_log(FUSE_LOG_DEBUG, "Func: lookup\n\tparent: %d\n\tname:%s\n", parent,
             name);
  }

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

  std::string local_fname;

  if (e.ino == FUSE_ROOT_ID) {
    local_fname = "";
  } else {
    local_fname = std::to_string(e.ino);
  }

  if (fstatat(context->get_root_fd(), local_fname.c_str(), &e.attr,
              AT_SYMLINK_NOFOLLOW | AT_EMPTY_PATH) == -1) {
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

  if (context->is_debug()) {
    fuse_log(FUSE_LOG_DEBUG, "Func: mknod\n\tparent: %d\n\tname:%s\n", parent,
             name);
  }

  tgfs_dir *parent_dir = context->lookup_dir(parent);
  if (parent_dir->contains(name)) {
    fuse_reply_err(req, EEXIST);
    return;
  }
  fuse_ino_t nod_ino = get_new_ino(*context);

  std::string local_fname = std::to_string(nod_ino);

  if (mknodat(context->get_root_fd(), local_fname.c_str(), mode, rdev) == -1) {
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

  struct fuse_entry_param e = {
      .ino = parent_dir->lookup(name),
      .attr_timeout = context->get_timeout(),
      .entry_timeout = context->get_timeout(),
  };

  if (fstatat(context->get_root_fd(), local_fname.c_str(), &e.attr, 0) == -1) {
    fuse_reply_err(req, errno);
    return;
  }
  e.attr.st_ino = e.ino;
  fuse_reply_entry(req, &e);
}

void tgfs_readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
                  struct fuse_file_info *fi) {
  tgfs_data *context = tgfs_data::tgfs_ptr(req);

  if (context->is_debug()) {
    fuse_log(FUSE_LOG_DEBUG,
             "Func: readdir\n\tinode: %d\n\tsize: %d\n\toffset: %d\n", ino,
             size, off);
  }

  const tgfs_dir *dir = context->lookup_dir(ino);
  char buf[size];
  char *p = buf;
  size_t rem = size;
  off_t nextoff = off;
  int err = 0;

  std::string local_fname;

  while (true) {
    size_t entsize;
    const char *name;
    const std::pair<fuse_ino_t, std::string> *ent;
    ent = dir->next(nextoff);
    if (ent == nullptr) {
      break;
    }
    nextoff = ent->first;

    if (ent->first == FUSE_ROOT_ID) {
      local_fname = "";
    } else {
      local_fname = std::to_string(ent->first);
    }

    struct stat st;
    if (fstatat(context->get_root_fd(), local_fname.c_str(), &st,
                AT_SYMLINK_NOFOLLOW | AT_EMPTY_PATH) == -1) {
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
  tgfs_data *context = tgfs_data::tgfs_ptr(req);
  int fd =
      openat(context->get_root_fd(), std::to_string(ino).c_str(), fi->flags);
  if (fd == -1) {
    fuse_reply_err(req, errno);
    return;
  }
  fi->fh = fd;
  fi->direct_io = 1;
  fuse_reply_open(req, fi);
}

void tgfs_read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
               struct fuse_file_info *fi) {
  struct fuse_bufvec buf = FUSE_BUFVEC_INIT(size);
  buf.buf[0].flags =
      static_cast<fuse_buf_flags>(FUSE_BUF_IS_FD | FUSE_BUF_FD_SEEK);
  buf.buf[0].fd = fi->fh;
  buf.buf[0].pos = off;
  fuse_reply_data(req, &buf, FUSE_BUF_SPLICE_MOVE);
}

void tgfs_write_buf(fuse_req_t req, fuse_ino_t ino, struct fuse_bufvec *in_buf,
                    off_t off, struct fuse_file_info *fi) {
  tgfs_data *context = tgfs_data::tgfs_ptr(req);
  size_t bufsize = fuse_buf_size(in_buf);
  if (off + bufsize > context->get_max_filesize()) {
    fuse_reply_err(req, EFBIG);
    return;
  }
  struct fuse_bufvec out_buf = FUSE_BUFVEC_INIT(bufsize);
  out_buf.buf[0].flags =
      static_cast<fuse_buf_flags>(FUSE_BUF_IS_FD | FUSE_BUF_FD_SEEK);
  out_buf.buf[0].fd = fi->fh;
  out_buf.buf[0].pos = off;

  ssize_t res =
      fuse_buf_copy(&out_buf, in_buf, static_cast<fuse_buf_copy_flags>(0));
  if (res < 0)
    fuse_reply_err(req, errno);
  else
    fuse_reply_write(req, (size_t)res);
}

void tgfs_flush(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
  tgfs_data *context = tgfs_data::tgfs_ptr(req);
  int err = context->upload(ino);
  fuse_reply_err(req, err);
}

void tgfs_release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
  int err = close(fi->fh);
  fuse_reply_err(req, err);
}

void tgfs_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
  tgfs_data *context = tgfs_data::tgfs_ptr(req);

  if (context->is_debug()) {
    fuse_log(FUSE_LOG_DEBUG, "Func: getattr\n\tinode: %d\n", ino);
  }

  std::string local_fname;

  if (ino == FUSE_ROOT_ID) {
    local_fname = "";
  } else {
    local_fname = std::to_string(ino);
  }

  struct stat st;
  if (fstatat(context->get_root_fd(), local_fname.c_str(), &st,
              AT_SYMLINK_NOFOLLOW | AT_EMPTY_PATH) == -1) {
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
    .flush = tgfs_flush,
    .release = tgfs_release,
    .readdir = tgfs_readdir,
    .write_buf = tgfs_write_buf,
};
