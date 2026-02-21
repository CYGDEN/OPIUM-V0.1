#pragma once
#include <string>
#include <vector>

class Crypto {
public:
    // SHA256 хэш пароля + соль
    static std::string hashPassword(const std::string& password,
                                    const std::string& salt);

    // Генерация соли
    static std::string generateSalt();

    // Проверка пароля
    static bool verifyPassword(const std::string& password,
                               const std::string& salt,
                               const std::string& hash);

    // Генерация случайного токена сессии
    static std::string generateToken();
};