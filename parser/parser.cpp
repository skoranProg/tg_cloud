#include "parser.h"

#include <iostream>
#include <ostream>


void Parser::parse() {
    for (const auto &s : stop_options) {
        if (find_option(s)) {
            tgcl_argv_.push_back(*find_option(s));
        }
    }
    char** cache_dir = find_option("--cache_dir", true);
    if (!cache_dir) {
        std::cerr << "No cache directory provided" << std::endl;
        return;
    }
    cache_dir_ = *(cache_dir + 1);


    char** api_id = find_option("-api_id", true);
    char** api_hash = find_option("-api_hash", true);
    if (api_hash == nullptr || api_id == nullptr) {
        // Treat errors
    } else {
        tgcl_argv_.push_back(*(api_id + 1));
        tgcl_argv_.push_back(*(api_hash + 1));
        tgcl_argv_.push_back(*(cache_dir + 1));
    }
    int current_size = 0;
    for (auto it = argv_; it != argv_ + argc_; it++) {
        if (it != api_id && it != api_hash && it != api_id + 1 && it != api_hash + 1 && it != cache_dir && it != cache_dir + 1) {
            tgfs_argv_.push_back(*it);
        }
    }
}

Parser::options Parser::get_tgfs_options() {
    return {static_cast<int>(tgfs_argv_.size()), tgfs_argv_.data()};
}

Parser::options Parser::get_tgcl_options() {
    return {static_cast<int>(tgcl_argv_.size()), tgcl_argv_.data()};
}

char** Parser::find_option(const std::string& option, bool not_last) {
    auto op = std::find(argv_, argv_ + argc_, option);
    if (op != argv_ + argc_ && (op + 1 != argv_ + argc_ || !not_last)) {
        return op;
    }
    return nullptr;
}

std::string Parser::get_cache_dir() const {
    return cache_dir_;
}
