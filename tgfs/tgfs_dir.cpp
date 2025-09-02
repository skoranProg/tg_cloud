#include "tgfs_dir.h"

tgfs_dir::tgfs_dir(const std::string &root_path, fuse_ino_t self)
    : tgfs_inode{(struct stat){.st_dev = 0,
                               .st_ino = self,
                               .st_nlink = 1,
                               .st_mode = S_IFDIR | S_IRWXU,
                               .st_uid = 0,
                               .st_gid = 0,
                               .st_rdev = 0,
                               .st_size = 666,
                               .st_blksize = 0,
                               .st_blocks = 1,
                               .st_atim = {},
                               .st_mtim = {},
                               .st_ctim = {}},
                 (uint64_t)0},
      tgfs_table<std::string, fuse_ino_t>{
          std::format("{}{}/data_0", root_path, self)} {}

int tgfs_dir::init(fuse_ino_t parent_dir) {
    tgfs_table<std::string, fuse_ino_t>::init();
    set(".", attr.st_ino);
    set("..", parent_dir);
    return 0;
}

std::vector<std::tuple<uint64_t, std::string, fuse_ino_t>> tgfs_dir::next(
    uint64_t off, int n) const {
    std::vector<std::tuple<uint64_t, std::string, fuse_ino_t>> res;
    res.reserve(n);
    char *err;

    std::clog << "tgfs_dir::next()\n\toff: " << off << "\n\tn: " << n
              << std::endl;

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
                ->emplace_back(static_cast<uint64_t>(std::atoll(values[0])),
                               std::string(values[1]),
                               static_cast<fuse_ino_t>(std::atoll(values[2])));
            std::clog << "\tentry: " << (values[0]) << " " << values[1] << " "
                      << (values[2]) << std::endl;
            return 0;
        },
        &res, &err);
    if (err) {
        sqlite3_free(err);
    }
    return res;
}

int tgfs_dir::upload_data(tgfs_net_api *api, int n,
                          const std::string &root_path) {
    sync();
    return tgfs_inode::upload_data(api, n, root_path);
}

int tgfs_dir::update_data(tgfs_net_api *api, int n,
                          const std::string &root_path) {
    close();
    int err = tgfs_inode::update_data(api, n, root_path);
    open(std::format("{}{}/data_0", root_path, attr.st_ino));
    return err;
}
