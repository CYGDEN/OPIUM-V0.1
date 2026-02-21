#pragma once
#include <windows.h>
#include <string>
#include <functional>

#define ID_LOGIN_USER     101
#define ID_LOGIN_PASS     102
#define ID_BTN_LOGIN      103
#define ID_BTN_TO_REG     104
#define ID_REG_USER       201
#define ID_REG_NAME       202
#define ID_REG_PHONE      203
#define ID_REG_PASS       204
#define ID_REG_PASS2      205
#define ID_BTN_REGISTER   206
#define ID_BTN_TO_LOGIN   207
#define ID_STATUS         301
#define ID_MSG_LIST       401
#define ID_CHAT_LIST      402
#define ID_MSG_INPUT      403
#define ID_BTN_SEND       404
#define ID_USERNAME_LABEL 405

#define COLOR_BG      RGB(17,  27,  39)
#define COLOR_PANEL   RGB(22,  33,  62)
#define COLOR_INPUT   RGB(30,  39,  50)
#define COLOR_ACCENT  RGB(41, 182, 246)
#define COLOR_TEXT    RGB(255,255,255)
#define COLOR_SUBTEXT RGB(150,160,170)
#define COLOR_GREEN   RGB(77, 182, 112)
#define COLOR_RED     RGB(229, 57,  53)

using RegisterCallback = std::function<std::string(
    const std::string&,  // username
    const std::string&,  // display_name
    const std::string&,  // phone
    const std::string&)>; // password

using LoginCallback = std::function<std::string(
    const std::string&,  // username
    const std::string&)>; // password

using SendCallback = std::function<bool(const std::string&)>;

class Window {
private:
    HWND        hwnd_login;
    HWND        hwnd_register;
    HWND        hwnd_main;
    HINSTANCE   hInst;
    HFONT       hFont;
    HFONT       hFontBig;
    HFONT       hFontTitle;
    HBRUSH      brBg;
    HBRUSH      brPanel;
    HBRUSH      brInput;
    HBRUSH      brAccent;

    RegisterCallback    onRegister;
    LoginCallback       onLogin;
    SendCallback        onSend;

    static Window* inst;

    void createLoginWindow();
    void createRegisterWindow();
    void createMainWindow(const std::string& username);

    void setStatus(HWND parent, const std::string& text,
                   COLORREF color = COLOR_SUBTEXT);

    static LRESULT CALLBACK LoginProc(HWND,UINT,WPARAM,LPARAM);
    static LRESULT CALLBACK RegisterProc(HWND,UINT,WPARAM,LPARAM);
    static LRESULT CALLBACK MainProc(HWND,UINT,WPARAM,LPARAM);

    HWND makeEdit(HWND parent, int x, int y,
                  int w, int h, int id,
                  const char* placeholder = "",
                  bool password = false);

    HWND makeButton(HWND parent, int x, int y,
                    int w, int h, int id,
                    const char* text);

    HWND makeLabel(HWND parent, int x, int y,
                   int w, int h,
                   const char* text,
                   HFONT font = nullptr);

public:
    Window(HINSTANCE hInstance);
    ~Window();

    bool init();
    void run();

    void showLogin();
    void showRegister();
    void showMain(const std::string& username);

    void addMessage(const std::string& sender,
                    const std::string& text,
                    bool is_mine);

    void setRegisterCallback(RegisterCallback cb) { onRegister = cb; }
    void setLoginCallback(LoginCallback cb)        { onLogin    = cb; }
    void setSendMessageCallback(SendCallback cb)   { onSend     = cb; }
};