/*
 * Pokémon: Ash Horizon
 * Copyright (C) 2026 Pokémon: Ash Horizon contributors
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * Private development-only cheat interface. This file is excluded from
 * clean/public builds unless ASH_HORIZON_PRIVATE_BUILD is defined.
 */
#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <SDL3/SDL.h>
#include <string>

namespace ash_horizon {

class PrivateCheatMenu {
public:
    explicit PrivateCheatMenu(SDL_Window* window) : window_(window) {}

    bool attach() {
#ifdef ASH_HORIZON_PRIVATE_BUILD
        if (!window_) return false;
        const SDL_PropertiesID properties = SDL_GetWindowProperties(window_);
        HWND hwnd = static_cast<HWND>(
            SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
        if (!hwnd) return false;

        instance_ = this;
        previousProc_ = reinterpret_cast<WNDPROC>(
            SetWindowLongPtrW(hwnd, GWLP_WNDPROC,
                reinterpret_cast<LONG_PTR>(&PrivateCheatMenu::windowProc)));
        if (!previousProc_) return false;
        windowHandle_ = hwnd;

        HMENU menu = GetMenu(hwnd);
        if (!menu) menu = CreateMenu();
        HMENU cheats = CreatePopupMenu();
        if (!menu || !cheats) return false;
        AppendMenuW(cheats, MF_STRING, ID_OPEN_CHEAT, L"Enter Cheat Code...");
        AppendMenuW(cheats, MF_STRING, ID_ACTIVE_CHEATS, L"Active Cheats");
        AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(cheats), L"Cheats");
        SetMenu(hwnd, menu);
        DrawMenuBar(hwnd);
        return true;
#else
        return false;
#endif
    }

    bool samfr2xActivated() const { return samfr2xActivated_; }

private:
#ifdef ASH_HORIZON_PRIVATE_BUILD
    static constexpr UINT ID_OPEN_CHEAT = 41001;
    static constexpr UINT ID_ACTIVE_CHEATS = 41002;
    static constexpr int ID_CODE_EDIT = 41003;
    static constexpr int ID_ACTIVATE = 41004;
    static constexpr int ID_STATUS = 41005;

    SDL_Window* window_ = nullptr;
    HWND windowHandle_ = nullptr;
    HWND dialog_ = nullptr;
    HWND edit_ = nullptr;
    HWND status_ = nullptr;
    WNDPROC previousProc_ = nullptr;
    bool samfr2xActivated_ = false;
    inline static PrivateCheatMenu* instance_ = nullptr;

    static LRESULT CALLBACK windowProc(HWND hwnd, UINT message,
                                       WPARAM wParam, LPARAM lParam) {
        PrivateCheatMenu* self = instance_;
        if (self && message == WM_COMMAND) {
            const UINT id = LOWORD(wParam);
            if (id == ID_OPEN_CHEAT) {
                self->openCodeWindow();
                return 0;
            }
            if (id == ID_ACTIVE_CHEATS) {
                MessageBoxW(hwnd,
                    self->samfr2xActivated_ ? L"samfr2x is active."
                                             : L"No active cheats.",
                    L"Active Cheats", MB_OK | MB_ICONINFORMATION);
                return 0;
            }
        }
        if (self && self->previousProc_)
            return CallWindowProcW(self->previousProc_, hwnd, message, wParam, lParam);
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    static LRESULT CALLBACK dialogProc(HWND hwnd, UINT message,
                                       WPARAM wParam, LPARAM lParam) {
        PrivateCheatMenu* self = instance_;
        if (!self) return DefWindowProcW(hwnd, message, wParam, lParam);
        if (message == WM_COMMAND && LOWORD(wParam) == ID_ACTIVATE) {
            wchar_t code[128]{};
            GetWindowTextW(self->edit_, code, 128);
            const std::wstring entered(code);
            if (entered == L"samfr2x") {
                self->samfr2xActivated_ = true;
                SetWindowTextW(self->status_, L"Code recognized. Item/time effects are not connected yet.");
            } else {
                SetWindowTextW(self->status_, L"Unknown code. Check spelling.");
            }
            return 0;
        }
        if (message == WM_CLOSE) {
            DestroyWindow(hwnd);
            if (self) {
                self->dialog_ = nullptr;
                self->edit_ = nullptr;
                self->status_ = nullptr;
            }
            return 0;
        }
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    void openCodeWindow() {
        if (dialog_ && IsWindow(dialog_)) {
            ShowWindow(dialog_, SW_SHOWNORMAL);
            SetForegroundWindow(dialog_);
            return;
        }
        static bool registered = false;
        if (!registered) {
            WNDCLASSW wc{};
            wc.lpfnWndProc = &PrivateCheatMenu::dialogProc;
            wc.hInstance = GetModuleHandleW(nullptr);
            wc.lpszClassName = L"AshHorizonPrivateCheatWindow";
            wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
            wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
            if (!RegisterClassW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
                return;
            registered = true;
        }

        dialog_ = CreateWindowExW(
            WS_EX_DLGMODALFRAME, L"AshHorizonPrivateCheatWindow",
            L"Enter Cheat Code", WS_CAPTION | WS_SYSMENU,
            CW_USEDEFAULT, CW_USEDEFAULT, 360, 155,
            windowHandle_, nullptr, GetModuleHandleW(nullptr), nullptr);
        if (!dialog_) return;

        CreateWindowExW(0, L"STATIC", L"Enter a private development cheat:",
            WS_CHILD | WS_VISIBLE, 14, 14, 320, 20,
            dialog_, nullptr, GetModuleHandleW(nullptr), nullptr);
        edit_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            14, 40, 315, 25, dialog_,
            reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_CODE_EDIT)),
            GetModuleHandleW(nullptr), nullptr);
        CreateWindowExW(0, L"BUTTON", L"Activate",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            245, 75, 84, 26, dialog_,
            reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_ACTIVATE)),
            GetModuleHandleW(nullptr), nullptr);
        status_ = CreateWindowExW(0, L"STATIC", L"Not activated",
            WS_CHILD | WS_VISIBLE, 14, 80, 215, 22,
            dialog_, reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_STATUS)),
            GetModuleHandleW(nullptr), nullptr);
        ShowWindow(dialog_, SW_SHOW);
        SetFocus(edit_);
    }
#endif
};

} // namespace ash_horizon
#endif
