#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <fstream>
#include <map>

// Структура пользователя
struct User {
    uint32_t    id;
    std::string username;
    std::string display_name;
    std::string password_hash;
    std::string phone;
    uint64_t    created_at;
    bool        is_online;

    User() : id(0), created_at(0), is_online(false) {}
};

// Структура сообщения (для будущего)
struct ChatMessage {
    uint32_t    id;
    uint32_t    from_id;
    uint32_t    to_id;
    std::string text;
    uint64_t    timestamp;

    ChatMessage() : id(0), from_id(0), to_id(0), timestamp(0) {}
};

class Database {
private:
    std::string     db_path;
    std::string     msg_path;

    // Данные в памяти
    std::map<uint32_t, User>        users;
    std::vector<ChatMessage>        messages;
    uint32_t                        next_user_id;
    uint32_t                        next_msg_id;

    // Бинарный формат файла
    bool saveUsers();
    bool loadUsers();
    bool saveMessages();
    bool loadMessages();

    // Утилиты
    uint64_t getCurrentTime();

public:
    Database();
    ~Database();

    bool init(const std::string& path);

    // Работа с пользователями
    bool        createUser(const std::string& username,
                           const std::string& display_name,
                           const std::string& phone,
                           const std::string& password_hash);

    User*       findByUsername(const std::string& username);
    User*       findByPhone(const std::string& phone);
    User*       findById(uint32_t id);

    bool        usernameExists(const std::string& username);
    bool        phoneExists(const std::string& phone);

    bool        updateOnline(uint32_t id, bool online);

    // Работа с сообщениями
    bool        saveMessage(uint32_t from, uint32_t to,
                            const std::string& text);
    std::vector<ChatMessage> getMessages(uint32_t user1,
                                         uint32_t user2);

    // Геттеры
    size_t      getUserCount() const { return users.size(); }
    std::vector<User> getAllUsers();
};