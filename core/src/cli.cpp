#include "cli.hpp"
#include <iostream>
#include <sstream>

CLI::CLI(Database& db, ZmqBridge& bridge) : db_(db), bridge_(bridge) {}

void CLI::print_help() const {
    std::cout << "\n--- Football Caller CLI ---\n"
            << "Available commands:\n"
            << "  add-user <username> <phone>             - Creates a new organizer (npr. add-user david 0641234567)\n"
            << "  list-users                              - Lists all organizers\n"
            << "  add-contact <user_id> <name> <phone>    - Adds a player (max 22 players per organizer)\n"
            << "  list-contacts <user_id>                 - Prints the players for the given organizer\n"
            << "  remove-contact <user_id> <phone>        - Deletes players by phone number\n"
            << "  ping-bridge                             - Test ZeroMQ connection with Python bridge\n"
            << "  help                                    - Displays this help\n"
            << "  exit/quit                               - Exit the program\n\n";
}

void CLI::run() {
    print_help();
    std::string line;

    while(true) {
        std::cout << "foodball-caller> ";
        if (!std::getline(std::cin, line)) {
            break;
        }
        if (line == "exit" || line == "quit") {
            break;
        }
        if (!line.empty()) {
            handle_command(line);
        }
    }
}

void CLI::handle_command(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    if (cmd == "help") {
        print_help();
    } else if (cmd == "ping-bridge") {
        nlohmman::json cmd_json = {{"type", "PING"}};
        if (bridge_.send_command(cmd_json)) {
            std::cout << "Ping command sent to Python bridge.\n";
        } else {
            std::cout << "Failed to send ping command to Python bridge.\n";
        }
    } else if (cmd == "add-user") {
        std::string username, phone;
        if (!(iss >> username >> phone)) {
            std::cout << "Usage: add-user <username> <phone>\n";
            return;
        }
        int new_id = 0;
        if (db_.add_user(username, phone, new_id)) {
            std::cout << "User " << username << " created with ID: " << new_id << "\n";
        } else {
            std::cout << "Failed to create user (username or phone number already exist).\n";
        }
    } else if (cmd == "list-users") {
        auto users = db_.list_users();
        if (users.empty()) {
            std::cout << "The list of users is empty.\n";
            return;
        }

        for (const auto& u : users) {
            int count = db_.count_contacts(u.id);
            std::cout << u.username << ": (" << u.phone_number << ") - number of contacts: " << count << "/" << Database::MAX_CONTACTS_PER_USER << "\n";
        }
    } else if (cmd == "add-contact") {
        int user_id;
        std::string name, phone;
        if (!(iss >> user_id >> name >> phone)) {
            std::cout << "Usage: add-contact <user_id> <name> <phone>\n";
            return;
        }

        std::string err;
        if (db_.add_contact(user_id, name, phone, err)) {
            int count = db_.count_contacts(user_id);
            std::cout << "Contact '" << name << "' added (" << count << "/" << Database::MAX_CONTACTS_PER_USER << ")\n";
        } else {
            std::cout << err << "\n";
        }
    } else if (cmd == "list-contacts") {
        int user_id;
        if (!(iss >> user_id)) {
            std::cout << "Usage: list-contacts <user_id>\n";
            return;
        }

        auto contacts = db_.list_contacts(user_id);
        std::cout << "Contacts for user with ID " << user_id << " (" << contacts.size() << "/" << Database::MAX_CONTACTS_PER_USER << "):\n";
        for (const auto& c : contacts) {
            std::cout << "  - " << c.name << " : " << c.phone_number << "\n";
        }
    } else if (cmd == "remove-contact") {
        int user_id;
        std::string phone;
        if (!(iss >> user_id >> phone)) {
            std::cout << "Usage: remove-contact <user_id> <phone>\n";
            return;
        }

        if (db_.remove_contact(user_id, phone)) {
            std::cout << "Contact with number " << phone << " is deleted.\n";
        } else {
            std::cout << "Contact with number " << phone << " not found.\n";
        }
    } else {
        std::cout << "Unknown command: '" << cmd << "'. Type 'help' for a list of commands.\n";
    }
}