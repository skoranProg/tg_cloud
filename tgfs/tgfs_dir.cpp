#include "tgfs_dir.h"

tgfs_dir::tgfs_dir(const std::string &root_path, fuse_ino_t self,
                   fuse_ino_t parent)
    : tgfs_inode{(struct stat){.st_dev = 0,
                               .st_ino = self,
                               .st_nlink = 1,
                               .st_mode = S_IFDIR | S_IRWXU,
                               .st_uid = 0,
                               .st_gid = 0,
                               .st_rdev = 0,
                               .st_size = 0,
                               .st_blksize = 0,
                               .st_blocks = 1,
                               .st_atim = {},
                               .st_mtim = {},
                               .st_ctim = {}},
                 (uint64_t)0},
      tgfs_table<std::string, fuse_ino_t>{
          std::format("{}{}/0", root_path, self)} {
    init();
    set(".", self);
    set("..", parent);
}

std::vector<std::tuple<uint64_t, std::string, fuse_ino_t>> tgfs_dir::next(
    uint64_t off, int n) const {
    std::vector<std::tuple<uint64_t, std::string, fuse_ino_t>> res;
    res.reserve(n);
    char *err;
    sqlite3_exec(
        table_,
        std::format("SELECT rowid, my_key, my_value FROM my_table WHERE rowid "
                    "> {} LIMIT {};",
                    off, n)
            .c_str(),
        [](void *res, int n, char *values[], char *columns[]) {
            reinterpret_cast<
                std::vector<std::tuple<uint64_t, std::string, fuse_ino_t>> *>(
                res)
                ->emplace_back(*reinterpret_cast<uint64_t *>(columns[0]),
                               std::string(columns[1]),
                               *reinterpret_cast<fuse_ino_t *>(columns[2]));
            return 0;
        },
        &res, &err);
    if (err) {
        sqlite3_free(err);
    }
    return res;
}
