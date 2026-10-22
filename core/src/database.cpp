#include "database.hpp"
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <sstream>

Database::Database(const std::string &db_path) {
    if (sqlite3_open(db_path.c_str(), &db_) != SQLITE_OK) {
        std::string err = db_ ? sqlite3_errmsg(db_) : "Unknown SQLite error";
        if (db_) sqlite3_close(db_);
        throw std::runtime_error("Failed to open the database: " + err);
    }

    std::string err;
    execute_sql("PRAGMA foreign_keys = ON;", err);
}

Database::~Database() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool Database::execute_sql(const std::string &sql, std::string &out_error) {
    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        if (err_msg) {
            out_error = err_msg;
            sqlite3_free(err_msg);
        }
        return false;
    }
    return true;
}


bool Database::init_schema(const std::string &migration_file_path) {
    std::ifstream file(migration_file_path);
    if (!file.is_open()) {
        std::cerr << "[ERROR]: Migration file cannot be opened: " << migration_file_path << "\n";
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string err;
    if (!execute_sql(buffer.str(), err)) {
        std::cerr << "[ERROR]: Schema initialization failed for " << migration_file_path << ": " << err << "\n";
        return false;
    }
    return true;
}

bool Database::add_user(const std::string &username, const std::string &phone_number, int &out_user_id) {
    const char* sql = "INSERT INTO users(username, phone_number) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, phone_number.c_str(), -1, SQLITE_TRANSIENT);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    if (success) {
        out_user_id = static_cast<int>(sqlite3_last_insert_rowid(db_));
    } else {
        std::cerr << "[SQLite]: " << sqlite3_errmsg(db_) << "\n";
    }

    sqlite3_finalize(stmt);
    return success;
}

std::vector<User> Database::list_users() {
    std::vector<User> users;
    const char* sql = "SELECT id, username, phone_number, created_at FROM users;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return users;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        User u;
        u.id = sqlite3_column_int(stmt, 0);
        u.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        u.phone_number = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        u.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        users.push_back(u);
    }

    sqlite3_finalize(stmt);
    return users;
}

bool Database::user_exists(int user_id) {
    const char* sql = "SELECT 1 FROM users WHERE id = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, user_id);

    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);

    sqlite3_finalize(stmt);
    return exists;
}

bool Database::add_contact(int user_id, const std::string &name, const std::string &phone_number, std::string &out_error) {\
    if (!user_exists(user_id)) {
        out_error = "User with ID " + std::to_string(user_id) + " doesn't exist.";
        return false;
    }

    int current_count = count_contacts(user_id);
    if (current_count >= MAX_CONTACTS_PER_USER) {
        out_error = "The maximum limit of " + std::to_string(MAX_CONTACTS_PER_USER) + " contacts for this user has be reached.";
        return false;
    }

    const char* sql = "INSERT INTO contacts(user_id, name, phone_number) VALUES(?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        out_error = sqlite3_errmsg(db_);
        return false;
    }

    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, phone_number.c_str(), -1, SQLITE_TRANSIENT);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!success) {
        out_error = sqlite3_errmsg(db_);
    }

    sqlite3_finalize(stmt);
    return success;
}

bool Database::remove_contact(int user_id, const std::string &phone_number) {
    const char* sql = "DELETE FROM contacts WHERE user_id = ? AND phone_number = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_text(stmt, 2, phone_number.c_str(), -1, SQLITE_TRANSIENT);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE) && (sqlite3_changes(db_) > 0);

    sqlite3_finalize(stmt);
    return success;
}

std::vector<Contact> Database::list_contacts(int user_id) {
    std::vector<Contact> contacts;
    const char* sql = "SELECT id, user_id, name, phone_number FROM contacts WHERE user_id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return contacts;
    }

    sqlite3_bind_int(stmt, 1, user_id);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Contact c;
        c.id = sqlite3_column_int(stmt, 0);
        c.user_id = sqlite3_column_int(stmt, 1);
        c.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        c.phone_number = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        contacts.push_back(c);
    }

    sqlite3_finalize(stmt);
    return contacts;
}

int Database::count_contacts(int user_id) {
    const char* sql = "SELECT COUNT(*) FROM contacts WHERE user_id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return -1;
    }

    sqlite3_bind_int(stmt, 1, user_id);

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}