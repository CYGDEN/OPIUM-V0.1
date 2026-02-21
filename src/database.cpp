#include "database.h"
#include <windows.h>
#include <iostream>
#include <ctime>
#include <cstring>
#include <algorithm>

Database::Database()
    : next_user_id(1)
    , next_msg_id(1)
{}

Database::~Database() {
    saveUsers();
    saveMessages();
}

uint64_t Database::getCurrentTime() {
    return (uint64_t)time(nullptr);
}

bool Database::init(const std::string& path) {
    db_path  = path + "\\users.odb";
    msg_path = path + "\\messages.odb";

    // Создаём папку data если нет
    CreateDirectoryA(path.c_str(), NULL);

    // Загружаем если файл есть
    loadUsers();
    loadMessages();

    std::cout << "[DB] Loaded " << users.size()
              << " users\n";
    return true;
}

// ============================================================
//  Бинарный формат .odb
//  Header: OPIUMDB\0 + version(4) + count(4)
//  User:   id(4) + username_len(2) + username
//                + name_len(2)     + name
//                + phone_len(2)    + phone
//                + hash_len(2)     + hash
//                + created_at(8)
// ============================================================
bool Database::saveUsers() {
    std::ofstream f(db_path, std::ios::binary);
    if (!f.is_open()) return false;

    // Заголовок
    const char header[] = "OPIUMDB\0";
    f.write(header, 8);

    uint32_t version = 1;
    f.write((char*)&version, 4);

    uint32_t count = (uint32_t)users.size();
    f.write((char*)&count, 4);

    // Записываем каждого пользователя
    for (auto& pair : users) {
        User& u = pair.second;

        f.write((char*)&u.id, 4);

        auto writeStr = [&](const std::string& s) {
            uint16_t len = (uint16_t)s.size();
            f.write((char*)&len, 2);
            f.write(s.c_str(), len);
        };

        writeStr(u.username);
        writeStr(u.display_name);
        writeStr(u.phone);
        writeStr(u.password_hash);

        f.write((char*)&u.created_at, 8);
    }

    // Сохраняем next_id
    f.write((char*)&next_user_id, 4);

    return true;
}

bool Database::loadUsers() {
    std::ifstream f(db_path, std::ios::binary);
    if (!f.is_open()) return false;

    // Проверяем заголовок
    char header[8];
    f.read(header, 8);
    if (strncmp(header, "OPIUMDB", 7) != 0) return false;

    uint32_t version;
    f.read((char*)&version, 4);

    uint32_t count;
    f.read((char*)&count, 4);

    auto readStr = [&]() -> std::string {
        uint16_t len;
        f.read((char*)&len, 2);
        if (len == 0) return "";
        std::string s(len, '\0');
        f.read(&s[0], len);
        return s;
    };

    users.clear();

    for (uint32_t i = 0; i < count; i++) {
        User u;
        f.read((char*)&u.id, 4);

        u.username      = readStr();
        u.display_name  = readStr();
        u.phone         = readStr();
        u.password_hash = readStr();

        f.read((char*)&u.created_at, 8);

        u.is_online = false;
        users[u.id] = u;
    }

    if (f.good()) {
        f.read((char*)&next_user_id, 4);
    }

    return true;
}

bool Database::saveMessages() {
    std::ofstream f(msg_path, std::ios::binary);
    if (!f.is_open()) return false;

    const char header[] = "OPIUMMS\0";
    f.write(header, 8);

    uint32_t count = (uint32_t)messages.size();
    f.write((char*)&count, 4);

    for (auto& m : messages) {
        f.write((char*)&m.id,        4);
        f.write((char*)&m.from_id,   4);
        f.write((char*)&m.to_id,     4);
        f.write((char*)&m.timestamp, 8);

        uint16_t len = (uint16_t)m.text.size();
        f.write((char*)&len, 2);
        f.write(m.text.c_str(), len);
    }

    f.write((char*)&next_msg_id, 4);
    return true;
}

bool Database::loadMessages() {
    std::ifstream f(msg_path, std::ios::binary);
    if (!f.is_open()) return false;

    char header[8];
    f.read(header, 8);
    if (strncmp(header, "OPIUMMS", 7) != 0) return false;

    uint32_t count;
    f.read((char*)&count, 4);

    messages.clear();

    for (uint32_t i = 0; i < count; i++) {
        ChatMessage m;
        f.read((char*)&m.id,        4);
        f.read((char*)&m.from_id,   4);
        f.read((char*)&m.to_id,     4);
        f.read((char*)&m.timestamp, 8);

        uint16_t len;
        f.read((char*)&len, 2);
        m.text.resize(len);
        f.read(&m.text[0], len);

        messages.push_back(m);
    }

    if (f.good()) {
        f.read((char*)&next_msg_id, 4);
    }

    return true;
}

// ============================================================
//  Создание пользователя
// ============================================================
bool Database::createUser(const std::string& username,
                          const std::string& display_name,
                          const std::string& phone,
                          const std::string& password_hash)
{
    if (usernameExists(username)) return false;
    if (phoneExists(phone))       return false;

    User u;
    u.id            = next_user_id++;
    u.username      = username;
    u.display_name  = display_name;
    u.phone         = phone;
    u.password_hash = password_hash;
    u.created_at    = getCurrentTime();
    u.is_online     = false;

    users[u.id] = u;

    saveUsers();

    std::cout << "[DB] User created: " << username
              << " (id=" << u.id << ")\n";
    return true;
}

User* Database::findByUsername(const std::string& username) {
    for (auto& pair : users) {
        if (pair.second.username == username)
            return &pair.second;
    }
    return nullptr;
}

User* Database::findByPhone(const std::string& phone) {
    for (auto& pair : users) {
        if (pair.second.phone == phone)
            return &pair.second;
    }
    return nullptr;
}

User* Database::findById(uint32_t id) {
    auto it = users.find(id);
    if (it != users.end()) return &it->second;
    return nullptr;
}

bool Database::usernameExists(const std::string& username) {
    return findByUsername(username) != nullptr;
}

bool Database::phoneExists(const std::string& phone) {
    return findByPhone(phone) != nullptr;
}

bool Database::updateOnline(uint32_t id, bool online) {
    User* u = findById(id);
    if (!u) return false;
    u->is_online = online;
    return true;
}

bool Database::saveMessage(uint32_t from, uint32_t to,
                           const std::string& text)
{
    ChatMessage m;
    m.id        = next_msg_id++;
    m.from_id   = from;
    m.to_id     = to;
    m.text      = text;
    m.timestamp = getCurrentTime();

    messages.push_back(m);
    saveMessages();
    return true;
}

std::vector<ChatMessage> Database::getMessages(uint32_t user1,
                                                uint32_t user2)
{
    std::vector<ChatMessage> result;
    for (auto& m : messages) {
        if ((m.from_id == user1 && m.to_id == user2) ||
            (m.from_id == user2 && m.to_id == user1))
        {
            result.push_back(m);
        }
    }
    return result;
}

std::vector<User> Database::getAllUsers() {
    std::vector<User> result;
    for (auto& pair : users) {
        result.push_back(pair.second);
    }
    return result;
}