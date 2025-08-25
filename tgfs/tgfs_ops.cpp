#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <time.h>
#include <unistd.h>

#include "tgfs_data.h"
#include "tgfs_helpers.h"

void tgfs_lookup(fuse_req_t req, fuse_ino_t parent, const char *name) {
    tgfs_data *context = tgfs_data::tgfs_ptr(req);

    if (context->is_debug()) {
        fuse_log(FUSE_LOG_DEBUG, "Func: lookup\n\tparent: %u\n\tname:%s\n",
                 parent, name);
    }

    tgfs_dir *parent_dir = context->lookup_dir(parent);
    if (!parent_dir->contains(name)) {
        fuse_reply_err(req, ENOENT);
        return;
    }

    fuse_ino_t ino = parent_dir->at(name);
    struct fuse_entry_param e = {
        .ino = ino,
        .attr = context->lookup_inode(ino)->attr,
        .attr_timeout = context->get_timeout(),
        .entry_timeout = context->get_timeout(),
    };

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
        fuse_log(FUSE_LOG_DEBUG, "Func: mknod\n\tparent: %u\n\tname:%s\n",
                 parent, name);
    }

    tgfs_dir *parent_dir = context->lookup_dir(parent);
    if (parent_dir->contains(name)) {
        fuse_reply_err(req, EEXIST);
        return;
    }

    fuse_ino_t ino = context->new_ino();

    std::string local_fname = std::to_string(ino);
    if (mkdirat(context->get_root_fd(), local_fname.c_str(), S_IFDIR | 0777) ==
        -1) {
        fuse_reply_err(req, errno);
        return;
    }

    int fd =
        openat(context->get_root_fd(),
               std::format("{}/inode", local_fname).c_str(), O_CREAT | O_RDWR);
    ftruncate(fd, sizeof(tgfs_inode));
    tgfs_inode *ino_obj = reinterpret_cast<tgfs_inode *>(mmap(
        NULL, sizeof(tgfs_inode), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    close(fd);

    mknodat(context->get_root_fd(), std::format("{}/0", local_fname).c_str(),
            S_IFREG | O_RDWR, 0);

    struct fuse_entry_param e = {
        .ino = ino,
        .attr =
            {
                .st_dev = rdev,
                .st_ino = ino,
                .st_nlink = 1,
                .st_mode = mode,
                .st_uid = 0,
                .st_gid = 0,
                .st_rdev = 0,
                .st_size = 0,
                .st_blksize = 0,
                .st_blocks = 1,
                .st_atim = {},
                .st_mtim = {},
                .st_ctim = {},
            },
        .attr_timeout = context->get_timeout(),
        .entry_timeout = context->get_timeout(),
    };
    clock_gettime(CLOCK_REALTIME, &(e.attr.st_atim));
    e.attr.st_mtim = e.attr.st_atim;
    e.attr.st_ctim = e.attr.st_atim;
    new (ino_obj) tgfs_inode(e.attr, (uint64_t)0);

    if (context->upload(ino_obj) != 0) {
        // TODO : handle exception
    }
    parent_dir->set(name, ino);
    if (context->upload(parent) != 0) {
        // TODO : handle exception
    }

    fuse_reply_entry(req, &e);
}

void tgfs_readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
                  struct fuse_file_info *fi) {
    tgfs_data *context = tgfs_data::tgfs_ptr(req);

    if (context->is_debug()) {
        fuse_log(FUSE_LOG_DEBUG,
                 "Func: readdir\n\tinode: %u\n\tsize: %u\n\toffset: %u\n", ino,
                 size, off);
    }

    const tgfs_dir *dir = context->lookup_dir(ino);
    char buf[size];
    char *p = buf;
    size_t rem = size;
    off_t nextoff = off;

    while (true) {
        std::vector<std::tuple<uint64_t, std::string, fuse_ino_t>> ents =
            dir->next(nextoff, 1);
        size_t entsize;
        if (ents.empty()) {
            break;
        }
        for (const std::tuple<uint64_t, std::string, fuse_ino_t> &ent : ents) {
            tgfs_inode *entino = context->lookup_inode(std::get<2>(ent));
            nextoff = std::get<0>(ent);
            entsize = fuse_add_direntry(req, p, rem, std::get<1>(ent).c_str(),
                                        &(entino->attr), nextoff);
            if (entsize > rem) {
                break;
            }
            p += entsize;
            rem -= entsize;
        }
        if (entsize > rem) {
            break;
        }
    }

    fuse_reply_buf(req, buf, size - rem);
}

void tgfs_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi) {
    tgfs_data *context = tgfs_data::tgfs_ptr(req);
    tgfs_inode *ino_obj = context->lookup_inode(ino);
    int fd = openat(context->get_root_fd(), std::format("{}/0", ino).c_str(),
                    fi->flags & (~O_TRUNC));
    if (fd == -1) {
        fuse_reply_err(req, errno);
        return;
    }
    fi->fh = fd;
    fi->direct_io = 1;

    if (fi->flags & O_TRUNC) {
        ftruncate(fd, sizeof(tgfs_inode));
        ino_obj->attr.st_size = 0;
    }

    fuse_reply_open(req, fi);
}

void tgfs_read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off,
               struct fuse_file_info *fi) {
    struct fuse_bufvec buf = FUSE_BUFVEC_INIT(size);
    buf.buf[0].flags =
        static_cast<fuse_buf_flags>(FUSE_BUF_IS_FD | FUSE_BUF_FD_SEEK);
    buf.buf[0].fd = fi->fh;
    buf.buf[0].pos = 0;
    buf.off = off;

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
    out_buf.buf[0].pos = 0;
    out_buf.off = off;

    ssize_t res =
        fuse_buf_copy(&out_buf, in_buf, static_cast<fuse_buf_copy_flags>(0));
    if (res < 0) {
        fuse_reply_err(req, errno);
    } else {
        tgfs_inode *ino_obj = context->lookup_inode(ino);
        ino_obj->attr.st_size =
            std::max(ino_obj->attr.st_size, static_cast<off_t>(off + res));
        clock_gettime(CLOCK_REALTIME, &(ino_obj->attr.st_mtim));
        fuse_reply_write(req, (size_t)res);
    }
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
        fuse_log(FUSE_LOG_DEBUG, "Func: getattr\n\tinode: %u\n", ino);
    }

    fuse_reply_attr(req, &(context->lookup_inode(ino)->attr),
                    context->get_timeout());
}

void tgfs_setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr, int to_set,
                  struct fuse_file_info *fi) {
    tgfs_data *context = tgfs_data::tgfs_ptr(req);

    if (context->is_debug()) {
        fuse_log(FUSE_LOG_DEBUG, "Func: setattr\n\tinode: %u\n\tto_set: %06o\n",
                 ino, to_set);
    }

    tgfs_inode *ino_obj = context->lookup_inode(ino);
    if (to_set & FUSE_SET_ATTR_MODE) {
        if (context->is_debug()) {
            fuse_log(FUSE_LOG_DEBUG, "\tmode: %07o\n", attr->st_mode);
        }

        ino_obj->attr.st_mode = attr->st_mode;
    }
    if (to_set & FUSE_SET_ATTR_UID) {
        if (context->is_debug()) {
            fuse_log(FUSE_LOG_DEBUG, "\tuid: %07o\n", attr->st_uid);
        }

        ino_obj->attr.st_uid = attr->st_uid;
    }
    if (to_set & FUSE_SET_ATTR_GID) {
        if (context->is_debug()) {
            fuse_log(FUSE_LOG_DEBUG, "\tgid: %07o\n", attr->st_gid);
        }

        ino_obj->attr.st_gid = attr->st_gid;
    }
    if (to_set & FUSE_SET_ATTR_SIZE) {
        if (context->is_debug()) {
            fuse_log(FUSE_LOG_DEBUG, "\tsize: %u\n", attr->st_size);
        }

        if (fi != NULL) {
            if (context->is_debug()) {
                fuse_log(FUSE_LOG_DEBUG, "\ttruncate fd: %u\n", fi->fh);
            }

            ftruncate(fi->fh, attr->st_size);
        }
        ino_obj->attr.st_size = attr->st_size;
    }
    if (to_set & FUSE_SET_ATTR_ATIME) {
        if (context->is_debug()) {
            fuse_log(FUSE_LOG_DEBUG, "\tatim\n");
        }

        ino_obj->attr.st_atim = attr->st_atim;
    }
    if (to_set & FUSE_SET_ATTR_MTIME) {
        if (context->is_debug()) {
            fuse_log(FUSE_LOG_DEBUG, "\tmtim\n");
        }

        ino_obj->attr.st_mtim = attr->st_mtim;
    }
    if (to_set & FUSE_SET_ATTR_ATIME_NOW) {
        // TODO
    }
    if (to_set & FUSE_SET_ATTR_MTIME_NOW) {
        // TODO
    }
    if (to_set & FUSE_SET_ATTR_FORCE) {
        // TODO
    }
    if (to_set & FUSE_SET_ATTR_CTIME) {
        if (context->is_debug()) {
            fuse_log(FUSE_LOG_DEBUG, "\tctim\n");
        }

        ino_obj->attr.st_ctim = attr->st_ctim;
    }
    if (to_set & FUSE_SET_ATTR_KILL_SUID) {
        // TODO
    }
    if (to_set & FUSE_SET_ATTR_KILL_SGID) {
        // TODO
    }
    if (to_set & FUSE_SET_ATTR_FILE) {
        // TODO
    }
    if (to_set & FUSE_SET_ATTR_KILL_PRIV) {
        // TODO
    }
    if (to_set & FUSE_SET_ATTR_OPEN) {
        // TODO
    }
    if (to_set & FUSE_SET_ATTR_TIMES_SET) {
        // TODO
    }
    if (to_set & FUSE_SET_ATTR_TOUCH) {
        // TODO
    }
    fuse_reply_attr(req, &(ino_obj->attr), context->get_timeout());
}

struct fuse_lowlevel_ops tgfs_opers = {
    .lookup = tgfs_lookup,
    .getattr = tgfs_getattr,
    .setattr = tgfs_setattr,
    .mknod = tgfs_mknod,
    .open = tgfs_open,
    .read = tgfs_read,
    .flush = tgfs_flush,
    .release = tgfs_release,
    .readdir = tgfs_readdir,
    .write_buf = tgfs_write_buf,
};
