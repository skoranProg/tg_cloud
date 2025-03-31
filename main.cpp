#include "tdclient.h"

#include "tgfs/tgfs.h"
#include "tgfs/tgfs_data.h"

/* Api_id and api_hash should be passed as program args */

/// TODO: Implement multithread process_update

#include <iostream>
#include <string.h>
#include <sqlite3.h>
#include <filesystem>

int main(int argc, char *argv[]) {
    //return make_new_tgfs(argc, argv, nullptr);
    TdClass td_client(std::stoi(argv[1]), argv[2]);
    td_client.Start();
    td_client.SetMainChatId("tg_cloudfilesbot");



}