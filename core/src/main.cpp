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

        ZmqBridge bridge("tcp://127.0.0.1:5555", "tcp://127.0.0.1:5556");
        bridge.start_listening([](const nlohmann::json& event) {
            std::cout << "Received message from Python bridge: " << event.dump() << "\nfootball-caller> " << std::flush;
        });

        CLI cli(db, bridge);
        cli.run();

        bridge.stop();
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << "\n";
        return -1;
    }

    return 0;
}