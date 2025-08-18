#ifndef _TGFS_H_
#define _TGFS_H_

#include "stdint.h"
#include <string>

class tgfs_net_api {
  public:
    virtual uint64_t upload(const std::string &path);
    virtual int download(uint64_t msg, const std::string &path);
    virtual int remove(uint64_t msg);
    virtual int download_table(const std::string &path);
    virtual int upload_table(const std::string &path);
};

int make_new_tgfs(int argc, char *argv[], tgfs_net_api *api);

#endif
