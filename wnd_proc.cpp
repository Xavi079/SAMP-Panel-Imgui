#include "wnd_proc.hpp"
#include "panel.hpp"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

wndProc::wndProc() {
    hookWindowProc_.install(std::make_tuple(this, &wndProc::hookedWindowProc));
}

wndProc::~wndProc() {
    hookWindowProc_.remove();
}

LRESULT wndProc::hookedWindowProc(oWndProc orig, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (panel::menuOpen)
        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

    if (uMsg == WM_KEYUP && wParam == VK_INSERT) {
        panel::menuOpen ^= true;
        static auto toggleCursor = [](bool show) {
            if (show) {
                while (ShowCursor(TRUE) < 0);
            } else {
                while (ShowCursor(FALSE) >= 0);
            }
        };
        toggleCursor(panel::menuOpen);
    }

    return orig(hWnd, uMsg, wParam, lParam);
}
