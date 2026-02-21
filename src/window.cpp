#include "window.h"
#include <windowsx.h>
#include <commctrl.h>
#include <richedit.h>

Window* Window::inst = nullptr;

Window::Window(HINSTANCE hInstance)
    : hwnd_login(nullptr)
    , hwnd_register(nullptr)
    , hwnd_main(nullptr)
    , hInst(hInstance)
    , hFont(nullptr)
    , hFontBig(nullptr)
    , hFontTitle(nullptr)
    , brBg(nullptr)
    , brPanel(nullptr)
    , brInput(nullptr)
    , brAccent(nullptr)
{
    inst = this;
}

Window::~Window() {
    if (hFont)     DeleteObject(hFont);
    if (hFontBig)  DeleteObject(hFontBig);
    if (hFontTitle) DeleteObject(hFontTitle);
    if (brBg)      DeleteObject(brBg);
    if (brPanel)   DeleteObject(brPanel);
    if (brInput)   DeleteObject(brInput);
    if (brAccent)  DeleteObject(brAccent);
}

bool Window::init() {
    INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&icc);

    brBg     = CreateSolidBrush(COLOR_BG);
    brPanel  = CreateSolidBrush(COLOR_PANEL);
    brInput  = CreateSolidBrush(COLOR_INPUT);
    brAccent = CreateSolidBrush(COLOR_ACCENT);

    hFont      = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    hFontBig   = CreateFontA(20, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    hFontTitle = CreateFontA(32, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

    createLoginWindow();
    return hwnd_login != nullptr;
}

void Window::run() {
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void Window::showLogin() {
    if (hwnd_register) ShowWindow(hwnd_register, SW_HIDE);
    if (hwnd_main)     ShowWindow(hwnd_main, SW_HIDE);
    ShowWindow(hwnd_login, SW_SHOW);
    SetFocus(hwnd_login);
}

void Window::showRegister() {
    ShowWindow(hwnd_login, SW_HIDE);
    if (!hwnd_register) createRegisterWindow();
    ShowWindow(hwnd_register, SW_SHOW);
    SetFocus(hwnd_register);
}

void Window::showMain(const std::string& username) {
    ShowWindow(hwnd_login, SW_HIDE);
    if (hwnd_register) ShowWindow(hwnd_register, SW_HIDE);
    if (!hwnd_main) createMainWindow(username);
    ShowWindow(hwnd_main, SW_SHOW);
    SetFocus(hwnd_main);
}

HWND Window::makeEdit(HWND parent, int x, int y, int w, int h, int id,
                      const char* placeholder, bool password) {
    DWORD style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL;
    if (password) style |= ES_PASSWORD;

    HWND hwnd = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                                style, x, y, w, h, parent, (HMENU)(intptr_t)id,
                                hInst, nullptr);
    SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, TRUE);
    if (placeholder[0]) {
        SendMessage(hwnd, EM_SETCUEBANNER, TRUE, (LPARAM)placeholder);
    }
    return hwnd;
}

HWND Window::makeButton(HWND parent, int x, int y, int w, int h, int id,
                        const char* text) {
    HWND hwnd = CreateWindowExA(0, "BUTTON", text,
                                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                x, y, w, h, parent, (HMENU)(intptr_t)id,
                                hInst, nullptr);
    SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, TRUE);
    return hwnd;
}

HWND Window::makeLabel(HWND parent, int x, int y, int w, int h,
                       const char* text, HFONT font) {
    HWND hwnd = CreateWindowExA(0, "STATIC", text,
                                WS_CHILD | WS_VISIBLE | SS_LEFT,
                                x, y, w, h, parent, nullptr,
                                hInst, nullptr);
    SendMessage(hwnd, WM_SETFONT, (WPARAM)(font ? font : hFont), TRUE);
    return hwnd;
}

void Window::setStatus(HWND parent, const std::string& text, COLORREF color) {
    HWND status = GetDlgItem(parent, ID_STATUS);
    if (status) {
        SetWindowTextA(status, text.c_str());
        SendMessage(status, WM_SETFONT, (WPARAM)hFont, TRUE);
    }
}

void Window::addMessage(const std::string& sender, const std::string& text, bool is_mine) {
    HWND list = GetDlgItem(hwnd_main, ID_MSG_LIST);
    if (!list) return;

    std::string msg = sender + ": " + text;
    int idx = ListBox_AddString(list, msg.c_str());
    ListBox_SetCurSel(list, idx);
}

void Window::createLoginWindow() {
    WNDCLASSEXA wc = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = LoginProc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = brBg;
    wc.lpszClassName = "OPIUMLogin";

    RegisterClassExA(&wc);

    hwnd_login = CreateWindowExA(0, "OPIUMLogin", "OPIUM - Login",
                                 WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                                 CW_USEDEFAULT, CW_USEDEFAULT, 400, 500,
                                 nullptr, nullptr, hInst, nullptr);

    makeLabel(hwnd_login, 120, 40, 160, 50, "OPIUM", hFontTitle);
    makeLabel(hwnd_login, 100, 100, 200, 20, "Username", hFontBig);
    makeEdit(hwnd_login, 80, 130, 240, 30, ID_LOGIN_USER, "Enter username");
    makeLabel(hwnd_login, 100, 180, 200, 20, "Password", hFontBig);
    makeEdit(hwnd_login, 80, 210, 240, 30, ID_LOGIN_PASS, "Enter password", true);
    makeButton(hwnd_login, 80, 280, 240, 40, ID_BTN_LOGIN, "Login");
    makeButton(hwnd_login, 80, 340, 240, 40, ID_BTN_TO_REG, "Create Account");
    makeLabel(hwnd_login, 80, 400, 240, 30, "", hFont);

    ShowWindow(hwnd_login, SW_SHOW);
}

void Window::createRegisterWindow() {
    WNDCLASSEXA wc = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = RegisterProc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = brBg;
    wc.lpszClassName = "OPIUMRegister";

    RegisterClassExA(&wc);

    hwnd_register = CreateWindowExA(0, "OPIUMRegister", "OPIUM - Register",
                                    WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                                    CW_USEDEFAULT, CW_USEDEFAULT, 400, 600,
                                    nullptr, nullptr, hInst, nullptr);

    makeLabel(hwnd_register, 120, 20, 160, 50, "OPIUM", hFontTitle);
    makeLabel(hwnd_register, 100, 70, 200, 20, "Username", hFont);
    makeEdit(hwnd_register, 80, 95, 240, 28, ID_REG_USER, "Username");
    makeLabel(hwnd_register, 100, 135, 200, 20, "Display Name", hFont);
    makeEdit(hwnd_register, 80, 160, 240, 28, ID_REG_NAME, "Your name");
    makeLabel(hwnd_register, 100, 200, 200, 20, "Phone", hFont);
    makeEdit(hwnd_register, 80, 225, 240, 28, ID_REG_PHONE, "+1234567890");
    makeLabel(hwnd_register, 100, 265, 200, 20, "Password", hFont);
    makeEdit(hwnd_register, 80, 290, 240, 28, ID_REG_PASS, "Password", true);
    makeLabel(hwnd_register, 100, 330, 200, 20, "Confirm Password", hFont);
    makeEdit(hwnd_register, 80, 355, 240, 28, ID_REG_PASS2, "Confirm", true);
    makeButton(hwnd_register, 80, 410, 240, 40, ID_BTN_REGISTER, "Register");
    makeButton(hwnd_register, 80, 470, 240, 40, ID_BTN_TO_LOGIN, "Back to Login");
    makeLabel(hwnd_register, 80, 530, 240, 30, "", hFont);
}

void Window::createMainWindow(const std::string& username) {
    WNDCLASSEXA wc = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = MainProc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = brBg;
    wc.lpszClassName = "OPIUMMain";

    RegisterClassExA(&wc);

    hwnd_main = CreateWindowExA(0, "OPIUMMain", "OPIUM",
                                WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 900, 600,
                                nullptr, nullptr, hInst, nullptr);

    makeLabel(hwnd_main, 20, 10, 220, 30, username.c_str(), hFontBig);

    HWND list = CreateWindowExA(WS_EX_CLIENTEDGE, "LISTBOX", "",
                                WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
                                20, 50, 550, 400, hwnd_main, (HMENU)ID_MSG_LIST,
                                hInst, nullptr);
    SendMessage(list, WM_SETFONT, (WPARAM)hFont, TRUE);

    makeEdit(hwnd_main, 20, 470, 440, 30, ID_MSG_INPUT, "Type a message...");
    makeButton(hwnd_main, 480, 470, 90, 30, ID_BTN_SEND, "Send");

    ShowWindow(hwnd_main, SW_SHOW);
}

LRESULT CALLBACK Window::LoginProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wp;
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, COLOR_TEXT);
            return (LRESULT)inst->brBg;
        }
        case WM_COMMAND: {
            int id = LOWORD(wp);
            if (id == ID_BTN_LOGIN) {
                char user[256], pass[256];
                GetDlgItemTextA(hwnd, ID_LOGIN_USER, user, 256);
                GetDlgItemTextA(hwnd, ID_LOGIN_PASS, pass, 256);

                if (!user[0] || !pass[0]) {
                    inst->setStatus(hwnd, "Fill all fields!", COLOR_RED);
                    return 0;
                }

                std::string result = inst->onLogin(user, pass);
                if (result == "OK") {
                    inst->setStatus(hwnd, "", COLOR_GREEN);
                } else {
                    inst->setStatus(hwnd, result, COLOR_RED);
                }
            }
            else if (id == ID_BTN_TO_REG) {
                inst->showRegister();
            }
            return 0;
        }
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

LRESULT CALLBACK Window::RegisterProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wp;
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, COLOR_TEXT);
            return (LRESULT)inst->brBg;
        }
        case WM_COMMAND: {
            int id = LOWORD(wp);
            if (id == ID_BTN_REGISTER) {
                char user[256], name[256], phone[256], pass[256], pass2[256];
                GetDlgItemTextA(hwnd, ID_REG_USER, user, 256);
                GetDlgItemTextA(hwnd, ID_REG_NAME, name, 256);
                GetDlgItemTextA(hwnd, ID_REG_PHONE, phone, 256);
                GetDlgItemTextA(hwnd, ID_REG_PASS, pass, 256);
                GetDlgItemTextA(hwnd, ID_REG_PASS2, pass2, 256);

                if (!user[0] || !name[0] || !phone[0] || !pass[0]) {
                    inst->setStatus(hwnd, "Fill all fields!", COLOR_RED);
                    return 0;
                }

                if (strcmp(pass, pass2) != 0) {
                    inst->setStatus(hwnd, "Passwords don't match!", COLOR_RED);
                    return 0;
                }

                std::string result = inst->onRegister(user, name, phone, pass);
                if (result == "OK") {
                    inst->setStatus(hwnd, "Registered! Now login.", COLOR_GREEN);
                    SetTimer(hwnd, 1, 1500, nullptr);
                } else {
                    inst->setStatus(hwnd, result, COLOR_RED);
                }
            }
            else if (id == ID_BTN_TO_LOGIN) {
                inst->showLogin();
            }
            return 0;
        }
        case WM_TIMER:
            KillTimer(hwnd, 1);
            inst->showLogin();
            return 0;
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

LRESULT CALLBACK Window::MainProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX: {
            HDC hdc = (HDC)wp;
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, COLOR_TEXT);
            return (LRESULT)inst->brBg;
        }
        case WM_COMMAND: {
            int id = LOWORD(wp);
            if (id == ID_BTN_SEND) {
                char text[1024];
                GetDlgItemTextA(hwnd, ID_MSG_INPUT, text, 1024);
                if (text[0] && inst->onSend) {
                    inst->onSend(text);
                    inst->addMessage("You", text, true);
                    SetDlgItemTextA(hwnd, ID_MSG_INPUT, "");
                }
            }
            return 0;
        }
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}
