#pragma once

#include <array>

inline __declspec(dllexport) std::array<unsigned char, 256> previous;
inline __declspec(dllexport) std::array<unsigned char, 256> current;

struct keybind {
    const char* name; // Name of the action, for config file mapping
    unsigned char keycode; // Virtual key code
    bool hold; // Whether the keybind should be active when held, or only on the frame it is pressed
    [[nodiscard]] bool active() const;
};

inline bool keybind::active() const {
    // Checks if the keybind is active based on the current and previous keyboard state
    const int down = current[keycode] & 0xF0;
    return hold
       ? down
       : down && !(previous[keycode] & 0xF0);
}

inline __declspec(dllexport) std::array<keybind, 12> keybinds;

void ReadKeybindConfig();
