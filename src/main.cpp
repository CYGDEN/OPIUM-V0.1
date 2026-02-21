#include <windows.h>
#include "window.h"
#include "database.h"
#include "auth.h"

static Database* g_db     = nullptr;
static Auth*     g_auth   = nullptr;
static Window*   g_window = nullptr;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE,
                   LPSTR, int)
{
    // Инициализация БД
    g_db = new Database();
    g_db->init("data");

    g_auth   = new Auth(*g_db);
    g_window = new Window(hInstance);

    if (!g_window->init()) {
        MessageBoxA(NULL, "Window init failed!", "Error", MB_OK);
        return 1;
    }

    // ── Регистрация ──────────────────────────────────────
    g_window->setRegisterCallback([](
        const std::string& username,
        const std::string& display_name,
        const std::string& phone,
        const std::string& password) -> std::string
    {
        auto result = g_auth->registerUser(
            username, display_name, phone, password);

        switch (result) {
            case AuthResult::OK:
                return "OK";
            case AuthResult::USER_EXISTS:
                return "Username already taken!";
            case AuthResult::PHONE_EXISTS:
                return "Phone already registered!";
            case AuthResult::INVALID_INPUT:
                return "Invalid input data!";
            default:
                return "Registration error!";
        }
    });

    // ── Вход ─────────────────────────────────────────────
    g_window->setLoginCallback([](
        const std::string& username,
        const std::string& password) -> std::string
    {
        auto result = g_auth->loginUser(username, password);

        switch (result) {
            case AuthResult::OK: {
                User* u = g_auth->getCurrentUser();
                g_window->showMain(u ? u->display_name : username);
                return "OK";
            }
            case AuthResult::USER_NOT_FOUND:
                return "User not found!";
            case AuthResult::WRONG_PASSWORD:
                return "Wrong password!";
            default:
                return "Login error!";
        }
    });

    // ── Отправка сообщения ────────────────────────────────
    g_window->setSendMessageCallback([](
        const std::string& text) -> bool
    {
        // Локально пока - сохраняем в БД
        if (!g_auth->isLoggedIn()) return false;
        uint32_t my_id = g_auth->getSession().user_id;
        // to_id = 0 пока (общий чат, в V0.2 добавим личку)
        g_db->saveMessage(my_id, 0, text);
        return true;
    });

    g_window->run();

    // Выход
    g_auth->logout();

    delete g_window;
    delete g_auth;
    delete g_db;

    return 0;
}