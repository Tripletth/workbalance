#define WINVER _WIN32_WINNT_WIN10

#include <windows.h>
#include <commctrl.h>
#include "resource.h"

#include <memory>
#include "util.h"

namespace
{
namespace controls
{
std::unique_ptr<util::EditControl<3>> worktime;
std::unique_ptr<util::EditControl<3>> breaktime;
std::unique_ptr<util::Control> clock;
std::unique_ptr<util::Control> line1;
} // namespace controls

util::DigitalClock digitalClock;
} // namespace

namespace tray {
constexpr unsigned int id = 7777;
constexpr int MESSAGE = WM_USER + 1;
HICON icon = ::LoadIcon(NULL, IDI_SHIELD);
NOTIFYICONDATA data = {
    .cbSize = sizeof(NOTIFYICONDATA),
    .hWnd = 0,
    .uID = id,
    .uFlags = NIF_STATE | NIF_ICON | NIF_MESSAGE,
    .uCallbackMessage = tray::MESSAGE,
    .hIcon = icon,
    .dwState = NIS_HIDDEN,
    .dwStateMask = 0,
    .uTimeout = 0,
    .dwInfoFlags = NIIF_NONE
};
void add(HWND hwndDlg) {
    tray::data.hWnd = hwndDlg;
    ::Shell_NotifyIconA(NIM_ADD, &tray::data);
}
void remove(HWND hWndDlg) {
    ::Shell_NotifyIconA(NIM_DELETE, &tray::data);
}
} // namespace tray

namespace dialog {
void activate(HWND hwndDlg) {
    ::ShowWindow(hwndDlg, SW_SHOW);
    ::SetWindowPos(hwndDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE);
}
void alert(HWND hwndDlg) {
    static FLASHWINFO flashWInfo = {
        .cbSize = sizeof(FLASHWINFO),
        .hwnd = hwndDlg,
        .dwFlags = FLASHW_ALL,
        .uCount = 2
    };
    dialog::activate(hwndDlg);
    ::MessageBeep(MB_ICONERROR);
    ::FlashWindowEx(&flashWInfo);
}
void deactivate(HWND hwndDlg) {
    ::SetWindowPos(hwndDlg, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
    ::SendMessage(hwndDlg, WM_KILLFOCUS, 0, 0);
    ::ShowWindow(hwndDlg, SW_HIDE);
}
} // namespace dialog

namespace timer {
constexpr int ALERT = 1;
constexpr int UPDATE = 2;
void handle(HWND, UINT, UINT_PTR, DWORD);
void start(HWND hwndDlg, int id, int timeout) {
    ::KillTimer(hwndDlg, id);
    ::SetTimer(hwndDlg, id, timeout, timer::handle);
}
void handle(HWND hwndDlg, UINT arg1, UINT_PTR id, DWORD arg4) {
    ::KillTimer(hwndDlg, id);
    switch(id) {
    case timer::ALERT:
        dialog::alert(hwndDlg);
        timer::start(hwndDlg, timer::ALERT, 3 * 1000);
        break;
    case timer::UPDATE:
        digitalClock.tick();
        controls::clock->setText(digitalClock.toString());
        timer::start(hwndDlg, timer::UPDATE, 1 * 1000);
        break;
    default:
        break;
    }
}
} // namespace timer

namespace callback {
template <int N>
void button(HWND hwndDlg, util::EditControl<N>* edit, const char* const txt) {
    const int seconds = std::max(1, edit->getInt()) * 60;
    timer::start(hwndDlg, timer::ALERT, seconds * 1000);
    digitalClock.set(seconds);
    digitalClock.down();
    controls::line1->setText(txt);
    controls::clock->setText(digitalClock.toString());
    dialog::deactivate(hwndDlg);
}
BOOL CALLBACK dlgMain(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
    case WM_INITDIALOG: {
        controls::worktime = std::make_unique<util::EditControl<3>>(DLG_MAIN_WERKTIJD, hwndDlg);
        controls::breaktime = std::make_unique<util::EditControl<3>>(DLG_MAIN_PAUZETIJD, hwndDlg);
        controls::clock = std::make_unique<util::Control>(DLG_MAIN_TIME, hwndDlg);
        controls::line1 = std::make_unique<util::Control>(DLG_MAIN_LINE1_TXT, hwndDlg);
        controls::worktime->setText("60");
        controls::breaktime->setText("5");
        ::SetWindowLong(hwndDlg, GWL_STYLE, ::GetWindowLong(hwndDlg, GWL_STYLE) | WS_MINIMIZEBOX);
        tray::add(hwndDlg);
        timer::start(hwndDlg, timer::UPDATE, 1 * 1000);
        return true;
    }
    case WM_CLOSE:
        if (::MessageBox(hwndDlg, "Wil je echt afsluiten?", "Afsluiten", MB_YESNO | MB_ICONQUESTION) == IDYES) {
            ::DestroyWindow(hwndDlg);
            tray::remove(hwndDlg);
            ::EndDialog(hwndDlg, 0);
        }
        return true;
    case WM_COMMAND:
        switch(LOWORD(wParam)) {
        case DLG_MAIN_PAUZEER:
            callback::button(hwndDlg, controls::breaktime.get(), "Pauze duurt nog:");
            break;
        case DLG_MAIN_WERKEN:
            callback::button(hwndDlg, controls::worktime.get(), "Volgende pauze:");
            break;
        default:
            break;
        }
        return true;
    case tray::MESSAGE:
        if (LOWORD(lParam) == WM_LBUTTONDBLCLK) {
            dialog::activate(hwndDlg);
            return true;
        }
    case WM_SYSCOMMAND:
        if (wParam == SC_MINIMIZE) {
            dialog::deactivate(hwndDlg);
            return true;
        }
    }
    return false;
}
} // namespace callback

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    ::InitCommonControls();
    return ::DialogBox(hInstance, MAKEINTRESOURCE(DLG_MAIN), NULL, (DLGPROC)callback::dlgMain);
}
