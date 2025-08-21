//
// Created by marat on 8/22/25.
//

#ifndef PARSER_H
#define PARSER_H

#include <algorithm>
#include <string>
#include <vector>

class Parser {
public:
    Parser(int argc, char** argv) : argc_(argc), argv_(argv) {
        parse();
    }

    std::pair<int, char**> get_tgfs_options();

    std::pair<int, char**> get_tgcl_options();
private:

    char** find_option(const std::string& option);

    void parse();

    int argc_, tgfs_argc_, tgcl_argc_;
    char **argv_;
    std::vector<char*> tgfs_argv_, tgcl_argv_;

};

#endif //PARSER_H
