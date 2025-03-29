#include "tgfs.h"
#include "tgfs_data.h"

extern struct fuse_lowlevel_ops tgfs_opers;

int make_new_tgfs(int argc, char *argv[], TdClass *tdclient) {
  double timeout = 1;
  int root_fd = 1;
  tgfs_data *context = new tgfs_data(timeout, root_fd, tdclient);

  delete context;
  return 0;
}
