#include "parser.h"

void Parser::parse() {
    char** api_id = find_option("-api_id");
    char** api_hash = find_option("-api_hash");
    if (api_hash == nullptr || api_id == nullptr) {
        // Treat errors
        return;
    }
    tgcl_argc_ = 2;
    tgcl_argv_ = std::vector<char*>(tgcl_argc_);
    tgcl_argv_[0] = *(api_id + 1);
    tgcl_argv_[1] = *(api_hash + 1);
    tgfs_argc_ = argc_ - 2 * tgcl_argc_;
    tgfs_argv_ = std::vector<char*>(tgfs_argc_);
    int current_size = 0;
    for (auto it = argv_; it != argv_ + argc_; it++) {
        if (it != api_id && it != api_hash && it != api_id + 1 && it != api_hash + 1) {
            tgfs_argv_[current_size++] = *it;
        }
    }
}

std::pair<int, char **> Parser::get_tgfs_options() {
    return std::make_pair(tgfs_argc_, tgfs_argv_.data());
}

std::pair<int, char **> Parser::get_tgcl_options() {
    return std::make_pair(tgcl_argc_, tgcl_argv_.data());
}

char** Parser::find_option(const std::string& option) {
    auto op = std::find(argv_, argv_ + argc_, option);
    if (op != argv_ + argc_ && op + 1 != argv_ + argc_) {
        return op;
    }
    return nullptr;
}
