#pragma once

#include <vector>
#include <windows.h>

typedef struct keybind {
    const char *name;
    unsigned char keycode;
    bool hold;
    void (*callback)();
} keybind;

inline std::vector<keybind> keybinds(14);
inline std::vector<unsigned char> previous(256);
inline std::vector<unsigned char> current(256);

void ReadKeybindConfig();

bool TryActivate(byte kCode);
