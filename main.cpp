#include "tdclient.h"

/* Api_id and api_hash should be passed as program args */

int main(int argc, char *argv[]) {
    TdClass td_client(std::stoi(argv[1]), argv[2]);
    td_client.Loop();
}