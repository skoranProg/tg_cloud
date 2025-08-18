#ifndef _TGFS_H_
#define _TGFS_H_

#include "stdint.h"
#include <string>

class tgfs_net_api {
  public:
    virtual uint64_t upload(std::string path);
    virtual int download(uint64_t msg, std::string path);
    virtual int remove(uint64_t msg);
    virtual uint64_t get_last_message();
};

int make_new_tgfs(int argc, char *argv[], tgfs_net_api *api);

#endif
