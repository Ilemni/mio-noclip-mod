#pragma once
#include <vector>

#include "modding_api.h"

typedef struct keybind {
    const char *name;
    unsigned char keycode;
    bool hold;
    void (*callback)(f32x3 *);
} keybind;

inline std::vector<keybind> keybinds(14);
inline std::vector<unsigned char> previous(256);
inline std::vector<unsigned char> current(256);

void ReadKeybindConfig();
bool TryActivate(byte kCode, f32x3 *loc);