#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "resource.h"
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include "util.h"

namespace
{
HINSTANCE hInst;
const int TIMER_WERKEN = 1;
const int TIMER_PAUZE = 2;
const int TIMER_UPDATE = 3;

DigitalClock digitalClock;
}

void startTimer(HWND hwndDlg, int id, int timeout, TIMERPROC callback) {
    KillTimer(hwndDlg, id);
    SetTimer(hwndDlg, id, timeout, callback);
}

void alert(HWND hwndDlg) {
    static std::unique_ptr<FLASHWINFO> flashInfo;
    if (!flashInfo) {
        flashInfo = std::make_unique<FLASHWINFO>();
        flashInfo->cbSize = sizeof(FLASHWINFO);
        flashInfo->hwnd = hwndDlg;
        flashInfo->dwFlags = FLASHW_ALL;
        flashInfo->uCount = 3;
    }
    SetWindowPos(hwndDlg, HWND_TOPMOST, 0, 0, 0 , 0, SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE);
    MessageBeep(MB_ICONERROR);
    FlashWindowEx(flashInfo.get());
}

void deactivate(HWND hwndDlg) {
    SetWindowPos(hwndDlg, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
    SendMessage(hwndDlg, WM_KILLFOCUS, 0, 0);
}

void handleTimer(HWND hwndDlg, UINT arg1, UINT_PTR id, DWORD arg4) {
    KillTimer(hwndDlg, id);

    switch(id) {
    case TIMER_WERKEN:
    case TIMER_PAUZE:
        alert(hwndDlg);
        break;
    case TIMER_UPDATE: {
        digitalClock.tick();
        Control(DLG_MAIN_TIME, hwndDlg).setText(digitalClock.toString());
        startTimer(hwndDlg, TIMER_UPDATE, 1 * 1000, handleTimer);
        break;
    }
    default:
        break;
    }
}

void handleCommand(int command, HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    auto handle = [hwndDlg](auto id, auto timer, auto&& txt) {
        int seconds = std::max(1, EditControl<3>(id, hwndDlg).getInt()) * 60;
        startTimer(hwndDlg, timer, seconds * 1000, handleTimer);
        digitalClock.set(seconds);
        digitalClock.down();
        Control(DLG_MAIN_LINE1_TXT, hwndDlg).setText(txt);
        deactivate(hwndDlg);
    };
    switch(command) {
    case DLG_MAIN_PAUZEER:
        handle(DLG_MAIN_PAUZETIJD, TIMER_PAUZE, "Pauze duurt nog:");
        break;
    case DLG_MAIN_WERKEN:
        handle(DLG_MAIN_WERKTIJD, TIMER_WERKEN, "Volgende pauze:");
        break;
    default:
        break;
    }
}

BOOL CALLBACK DlgMain(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
    case WM_INITDIALOG: {
        EditControl<3>(DLG_MAIN_WERKTIJD, hwndDlg).setText("60");
        EditControl<3>(DLG_MAIN_PAUZETIJD, hwndDlg).setText("5");
        startTimer(hwndDlg, TIMER_UPDATE, 1 * 1000, handleTimer);
        return true;
    }
    case WM_CLOSE:
        EndDialog(hwndDlg, 0);
        return true;
    case WM_COMMAND:
        handleCommand(LOWORD(wParam), hwndDlg, uMsg, wParam, lParam);
        return true;
    }
    return false;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    hInst = hInstance;
    InitCommonControls();
    return DialogBox(hInst, MAKEINTRESOURCE(DLG_MAIN), NULL, (DLGPROC)DlgMain);
}
