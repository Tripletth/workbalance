#pragma once

#include <iomanip>
#include <sstream>
#include <string>
#include <windows.h>

namespace util {
class Control {
public:
    Control(int id, HWND parent) : id{id}, parent{parent} {}
    void setText(const std::string& str) const {
        ::SetWindowText(getHandle(), str.c_str());
    }
protected:
    HWND getHandle() const {
        return ::GetDlgItem(parent, id);
    }
private:
    const int id;
    const HWND parent;
};
template <int MaxLength = 1024> class EditControl : public Control {
public:
    EditControl(int id, HWND parent) : Control{id, parent} {
        ::SendMessage(getHandle(), EM_SETLIMITTEXT, MaxLength, 0);
    }
    std::string getString() const {
        char buffer[MaxLength + 1];
        HWND item = getHandle();
        std::size_t n = std::min(::GetWindowTextLength(item), MaxLength);
        ::GetWindowText(item, buffer, n + 1);
        return {buffer, n};
    }
    int getInt() const {
        try {
            return std::stoi(getString());
        } catch(std::invalid_argument& e) {
            return 0;
        } catch(std::out_of_range& e) {
            return 0;
        }
    }
};
class DigitalClock {
    enum class Mode { UP, DOWN, PAUSE };
public:
    DigitalClock() : mode{Mode::PAUSE}, seconds{0} {}
    void tick() {
        switch(mode) {
        case Mode::PAUSE:
            break;
        case Mode::UP:
            ++seconds;
            break;
        case Mode::DOWN:
            if (--seconds <= 0) {
                pause();
            }
            break;
        }
    }
    auto toString() const {
        const int hours = seconds / 3600;
        const int minutes = (seconds / 60) - (hours * 60);
        std::stringstream ss;
        ss << std::setfill('0')
           << std::setw(2) << hours << ':'
           << std::setw(2) << minutes << ':'
           << std::setw(2) << seconds % 60;
        return ss.str();
    }
    void set(int s) { seconds = s; }
    void down() { mode = Mode::DOWN; }
    void up() { mode = Mode::UP; }
    void pause() { mode = Mode::PAUSE; }
private:
    Mode mode;
    int seconds;
};
} // namespace util
