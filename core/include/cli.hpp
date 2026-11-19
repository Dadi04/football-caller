#ifndef CLI_HPP
#define CLI_HPP

#include "database.hpp"
#include <string>

class CLI {
public:
    CLI(Database& db, ZmqBridge& bridge);
    void run();
private:
    Database& db_;
    ZmqBridge& bridge_;
    void print_help() const;
    void handle_command(const std::string& line);
};

#endif