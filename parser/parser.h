#ifndef PARSER_H
#define PARSER_H

#include <algorithm>
#include <string>
#include <vector>

class Parser {
 public:
    struct options {
        int argc;
        char** argv;
    };

    Parser(int argc, char** argv) : argc_(argc), argv_(argv) {
        parse();
    }

    options get_tgfs_options();

    options get_tgcl_options();

    std::string get_cache_dir() const;

    std::pair<std::string, bool> get_key_path() const;

    bool information_option() const;

 private:
    char** find_option(const std::string& option, bool not_last = false);

    void parse();

    int argc_;
    char** argv_;
    std::vector<char*> tgfs_argv_, tgcl_argv_;
    char* cache_dir_ = nullptr;
    char* key_path_ = nullptr;
    bool has_information_option_ = false;
};

const std::vector<std::string> stop_options = {"--help", "--version", "-h",
                                               "-v", "-hv"};

#endif  // PARSER_H
