#pragma once
#include <string>
#include "database.h"

enum class AuthResult {
    OK,
    USER_EXISTS,
    PHONE_EXISTS,
    WRONG_PASSWORD,
    USER_NOT_FOUND,
    INVALID_INPUT,
    FAILURE
};

struct Session {
    uint32_t    user_id;
    std::string token;
    bool        active;

    Session() : user_id(0), active(false) {}
};

class Auth {
private:
    Database&   db;
    Session     current_session;

    bool validateUsername(const std::string& s);
    bool validatePhone(const std::string& s);
    bool validatePassword(const std::string& s);

public:
    Auth(Database& database);

    // Регистрация
    AuthResult  registerUser(const std::string& username,
                             const std::string& display_name,
                             const std::string& phone,
                             const std::string& password);

    // Вход
    AuthResult  loginUser(const std::string& username,
                          const std::string& password);

    // Выход
    void        logout();

    // Состояние
    bool        isLoggedIn() const { return current_session.active; }
    Session&    getSession()       { return current_session; }
    User*       getCurrentUser();
};