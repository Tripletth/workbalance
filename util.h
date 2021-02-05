#pragma once

#include <iomanip>
#include <sstream>

class Control {
public:
    Control(int id, HWND parent) : id{id}, parent{parent} {}
    void setText(const std::string& str) {
        SetWindowText(handle(), str.c_str());
    }
protected:
    HWND handle() {
        return GetDlgItem(parent, id);
    }
public:
    int id;
private:
    HWND parent;
};

template <int MaxSize = 1024>
class EditControl : public Control {
public:
    EditControl(int id, HWND parent) : Control{id, parent} {}
    std::string getString() {
        char buffer[MaxSize + 1];
        HWND item = handle();
        GetWindowText(item, buffer, MaxSize + 1);
        return {buffer, static_cast<std::size_t>(GetWindowTextLength(item))};
    }
    int getInt() {
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
            if (seconds == 0) {
                pause();
            } else {
                --seconds;
            }
            break;
        }
    }
    std::string toString() {
        int hours = seconds / 3600;
        int minutes = (seconds / 60) - (hours * 60);
        int mod_seconds = seconds % 60;
        std::stringstream ss;
        ss << std::setfill('0')
           << std::setw(2) << hours << ':'
           << std::setw(2) << minutes << ':'
           << std::setw(2) << mod_seconds;
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
