#include "tgcl/tdclient.h"

#include "tgfs/tgfs.h"
#include "tgfs/tgfs_data.h"

#include "parser/parser.h"

/* Api_id and api_hash should be passed as program args */

#include <iostream>
#include <string.h>
#include <sqlite3.h>
#include <filesystem>

#include "tgcl/tgfs_api.h"

int main(int argc, char *argv[]) {
    Parser parser(argc, argv);
    TdClass td_client = create_td_client(parser.get_tgcl_options().argc, parser.get_tgcl_options().argv);
    td_client_api td_api(&td_client);
    return make_new_tgfs(parser.get_tgfs_options().argc, parser.get_tgfs_options().argv, &td_api, parser.get_cache_dir());
}