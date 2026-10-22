#include "database.hpp"
#include "cli.hpp"
#include <exception>
#include <iostream>

int main() {
    try {
        Database db("football_caller.db");

        if (!db.init_schema("migrations/001_init.sql")) {
            std::cerr << "Program terminated due to database migration error.\n";
            return -1;
        }

        CLI cli(db);
        cli.run();
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << "\n";
        return -1;
    }

    return 0;
}