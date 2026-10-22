#ifndef CLI_HPP
#define CLI_HPP

#include "database.hpp"
#include <string>

class CLI {
public:
    CLI(Database& db);
    void run();
private:
    Database& db_;
    void print_help() const;
    void handle_command(const std::string& line);
};

#endif