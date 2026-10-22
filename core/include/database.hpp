#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <string>
#include <vector>
#include <sqlite3.h>

struct User {
    int id;
    std::string username;
    std::string phone_number;
    std::string created_at;
};

struct Contact {
    int id;
    int user_id;
    std::string name;
    std::string phone_number;
};

class Database {
public:
    static const int MAX_CONTACTS_PER_USER = 22;

    Database(const std::string& db_path);
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    bool init_schema(const std::string& migration_file_path);

    bool add_user(const std::string& username, const std::string& phone_number, int& out_user_id);
    std::vector<User> list_users();
    bool user_exists(int user_id);

    bool add_contact(int user_id, const std::string& name, const std::string& phone_number, std::string& out_error);
    bool remove_contact(int user_id, const std::string& phone_number);
    std::vector<Contact> list_contacts(int user_id);
    int count_contacts(int user_id);
private: 
    sqlite3* db_ = nullptr;
    bool execute_sql(const std::string& sql, std::string& out_error);
};

#endif