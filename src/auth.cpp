#include "auth.h"
#include "crypto.h"
#include <iostream>
#include <algorithm>
#include <cctype>

Auth::Auth(Database& database) : db(database) {}

bool Auth::validateUsername(const std::string& s) {
    if (s.size() < 3 || s.size() > 32) return false;
    for (char c : s) {
        if (!isalnum(c) && c != '_') return false;
    }
    return true;
}

bool Auth::validatePhone(const std::string& s) {
    if (s.size() < 7 || s.size() > 16) return false;
    size_t start = (s[0] == '+') ? 1 : 0;
    for (size_t i = start; i < s.size(); i++) {
        if (!isdigit(s[i])) return false;
    }
    return true;
}

bool Auth::validatePassword(const std::string& s) {
    return s.size() >= 6;
}

// ============================================================
//  Регистрация
// ============================================================
AuthResult Auth::registerUser(const std::string& username,
                               const std::string& display_name,
                               const std::string& phone,
                               const std::string& password)
{
    // Валидация
    if (!validateUsername(username))  return AuthResult::INVALID_INPUT;
    if (!validatePhone(phone))        return AuthResult::INVALID_INPUT;
    if (!validatePassword(password))  return AuthResult::INVALID_INPUT;
    if (display_name.empty())         return AuthResult::INVALID_INPUT;

    // Проверка уникальности
    if (db.usernameExists(username))  return AuthResult::USER_EXISTS;
    if (db.phoneExists(phone))        return AuthResult::PHONE_EXISTS;

    // Хэшируем пароль
    std::string salt = Crypto::generateSalt();
    std::string hash = Crypto::hashPassword(password, salt);

    // Сохраняем хэш как salt:hash
    std::string stored = salt + ":" + hash;

    if (!db.createUser(username, display_name, phone, stored)) {
        return AuthResult::FAILURE;
    }

    std::cout << "[Auth] Registered: " << username << "\n";
    return AuthResult::OK;
}

// ============================================================
//  Вход
// ============================================================
AuthResult Auth::loginUser(const std::string& username,
                            const std::string& password)
{
    User* user = db.findByUsername(username);
    if (!user) return AuthResult::USER_NOT_FOUND;

    // Разбираем salt:hash
    std::string stored = user->password_hash;
    size_t sep = stored.find(':');
    if (sep == std::string::npos) return AuthResult::FAILURE;

    std::string salt = stored.substr(0, sep);
    std::string hash = stored.substr(sep + 1);

    if (!Crypto::verifyPassword(password, salt, hash)) {
        return AuthResult::WRONG_PASSWORD;
    }

    // Создаём сессию
    current_session.user_id = user->id;
    current_session.token   = Crypto::generateToken();
    current_session.active  = true;

    db.updateOnline(user->id, true);

    std::cout << "[Auth] Login: " << username << "\n";
    return AuthResult::OK;
}

void Auth::logout() {
    if (current_session.active) {
        db.updateOnline(current_session.user_id, false);
    }
    current_session = Session();
}

User* Auth::getCurrentUser() {
    if (!current_session.active) return nullptr;
    return db.findById(current_session.user_id);
}