#ifndef _TGFS_H_
#define _TGFS_H_

#include <stdint.h>
#include <string>

class tgfs_net_api {
  public:
    virtual uint64_t upload(const std::string &path) = 0;
    virtual int download(uint64_t msg, const std::string &path) = 0;
    virtual int remove(uint64_t msg) = 0;

    virtual bool is_up_to_date_table() = 0;
    // Should return:
    // 0 - on successful download
    // 1 - if table already was up-to-date
    // 2 - if chat is empty
    // anything else - on error
    virtual int download_table(const std::string &path) = 0;
    virtual int upload_table(const std::string &path) = 0;
};

int make_new_tgfs(int argc, char *argv[], tgfs_net_api *api, const std::string& cache_dir);

#endif
