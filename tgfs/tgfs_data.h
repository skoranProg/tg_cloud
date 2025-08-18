#ifndef _TGFS_DATA_H_
#define _TGFS_DATA_H_

#include "tgfs_fuse_dependencies.h"

#include "tgfs.h"
#include "tgfs_dir.h"
#include "tgfs_inode.h"
#include "tgfs_table.h"

class tgfs_data {
  private:
    tgfs_net_api *api_;
    double timeout_;
    const int root_fd_;
    const std::string root_path_;
    const std::string table_path_;
    const size_t max_filesize_;
    bool debug_;

    // Only physically present(downloaded) files
    // May contain outdated information
    std::unordered_map<fuse_ino_t, tgfs_inode *> inodes_; // ino -> inode

    // All files on server
    // Must always be up-to-date(which means sync of whole table before each
    // call)
    tgfs_table<fuse_ino_t, uint64_t> messages_; // ino -> msg_id

    int update_table();

  public:
    tgfs_data(bool debug, double timeout, int root_fd, size_t max_filesize,
              tgfs_net_api *api);

    static tgfs_data *tgfs_ptr(fuse_req_t req);

    bool is_debug() const;

    double get_timeout() const;

    int get_root_fd() const;

    size_t get_max_filesize() const;

    uint64_t lookup_msg(fuse_ino_t ino);

    tgfs_inode *lookup_inode(fuse_ino_t ino);

    tgfs_dir *lookup_dir(fuse_ino_t ino);

    int upload(fuse_ino_t ino);
    int upload(tgfs_inode *ino);

    int update(fuse_ino_t ino);
};

#endif
