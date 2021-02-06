#include <windows.h>
#include <commctrl.h>
#include "resource.h"

#include <memory>
#include <sstream>
#include <string>
#include "util.h"
#include <vector>

namespace
{
namespace controls
{
std::unique_ptr<util::EditControl<3>> worktime;
std::unique_ptr<util::EditControl<3>> breaktime;
std::unique_ptr<util::Control> clock;
std::unique_ptr<util::Control> line1;
} // namespace controls

const int TIMER_ALERT = 1;
const int TIMER_UPDATE = 2;
util::DigitalClock digitalClock;
} // namespace

namespace dialog {
void alert(HWND hwndDlg) {
    static FLASHWINFO flashWInfo = {
        .cbSize = sizeof(FLASHWINFO),
        .hwnd = hwndDlg,
        .dwFlags = FLASHW_ALL,
        .uCount = 3
    };
    ::SetWindowPos(hwndDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE);
    ::MessageBeep(MB_ICONERROR);
    ::FlashWindowEx(&flashWInfo);
}

void deactivate(HWND hwndDlg) {
    ::SetWindowPos(hwndDlg, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
    ::SendMessage(hwndDlg, WM_KILLFOCUS, 0, 0);
}
} // namespace dialog

namespace timer {
void start(HWND hwndDlg, int id, int timeout, TIMERPROC callback) {
    ::KillTimer(hwndDlg, id);
    ::SetTimer(hwndDlg, id, timeout, callback);
}

void handle(HWND hwndDlg, UINT arg1, UINT_PTR id, DWORD arg4) {
    ::KillTimer(hwndDlg, id);
    switch(id) {
    case TIMER_ALERT:
        dialog::alert(hwndDlg);
        break;
    case TIMER_UPDATE:
        digitalClock.tick();
        controls::clock->setText(digitalClock.toString());
        timer::start(hwndDlg, TIMER_UPDATE, 1 * 1000, timer::handle);
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
    timer::start(hwndDlg, TIMER_ALERT, seconds * 1000, timer::handle);
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
        timer::start(hwndDlg, TIMER_UPDATE, 1 * 1000, timer::handle);
        return true;
    }
    case WM_CLOSE:
        ::EndDialog(hwndDlg, 0);
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
    }
    return false;
}
} // namespace callback

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    ::InitCommonControls();
    return ::DialogBox(hInstance, MAKEINTRESOURCE(DLG_MAIN), NULL, (DLGPROC)callback::dlgMain);
}
