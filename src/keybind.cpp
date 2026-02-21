#include "keybind.h"

#include "modding_api.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>

const auto keybind_config_dir = "modconfig/noclip/";
const auto keybind_config_path = "modconfig/noclip/keybinds.txt";

// Map of string representations to virtual key codes based on https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
// Contains duplicates for common keys (e.g. numpad0 and n0, multiply and *) to allow for more intuitive keybind configs
std::unordered_map<std::string, unsigned char> virtualKeyMap = {
    {"lmb", VK_LBUTTON},
    {"rmb", VK_RBUTTON},
    {"cancel", VK_CANCEL},
    {"mmb", VK_MBUTTON},
    {"x1mb", VK_XBUTTON1},
    {"x2mb", VK_XBUTTON2},
    {"backspace", VK_BACK},
    {"tab", VK_TAB},
    {"enter", VK_RETURN},
    {"shift", VK_SHIFT},
    {"ctrl", VK_CONTROL},
    {"alt", VK_MENU},
    {"capslock", VK_CAPITAL},
    {"esc", VK_ESCAPE},
    {"space", VK_SPACE},
    {"pageup", VK_PRIOR},
    {"pagedown", VK_NEXT},
    {"end", VK_END},
    {"home", VK_HOME},
    {"left", VK_LEFT},
    {"right", VK_RIGHT},
    {"up", VK_UP},
    {"down", VK_DOWN},
    {"select", VK_SELECT},
    {"print", VK_PRINT},
    {"snapshot", VK_SNAPSHOT},
    {"insert", VK_INSERT},
    {"ins", VK_INSERT},
    {"delete", VK_DELETE},
    {"del", VK_DELETE},
    {"0", 0x30},
    {"1", 0x31},
    {"2", 0x32},
    {"3", 0x33},
    {"4", 0x34},
    {"5", 0x35},
    {"6", 0x36},
    {"7", 0x37},
    {"8", 0x38},
    {"9", 0x39},
    {"a", 0x41},
    {"b", 0x42},
    {"c", 0x43},
    {"d", 0x44},
    {"e", 0x45},
    {"f", 0x46},
    {"g", 0x47},
    {"h", 0x48},
    {"i", 0x49},
    {"j", 0x4A},
    {"k", 0x4B},
    {"l", 0x4C},
    {"m", 0x4D},
    {"n", 0x4E},
    {"o", 0x4F},
    {"p", 0x50},
    {"q", 0x51},
    {"r", 0x52},
    {"s", 0x53},
    {"t", 0x54},
    {"u", 0x55},
    {"v", 0x56},
    {"w", 0x57},
    {"x", 0x58},
    {"y", 0x59},
    {"z", 0x5A},
    {"lwin", VK_LWIN},
    {"rwin", VK_RWIN},
    {"apps", VK_APPS},
    {"numpad0", VK_NUMPAD0},
    {"numpad1", VK_NUMPAD1},
    {"numpad2", VK_NUMPAD2},
    {"numpad3", VK_NUMPAD3},
    {"numpad4", VK_NUMPAD4},
    {"numpad5", VK_NUMPAD5},
    {"numpad6", VK_NUMPAD6},
    {"numpad7", VK_NUMPAD7},
    {"numpad8", VK_NUMPAD8},
    {"numpad9", VK_NUMPAD9},
    {"n0", VK_NUMPAD0}, // Duplicates for shorthand n0-9
    {"n1", VK_NUMPAD1},
    {"n2", VK_NUMPAD2},
    {"n3", VK_NUMPAD3},
    {"n4", VK_NUMPAD4},
    {"n5", VK_NUMPAD5},
    {"n6", VK_NUMPAD6},
    {"n7", VK_NUMPAD7},
    {"n8", VK_NUMPAD8},
    {"n9", VK_NUMPAD9},
    {"multiply", VK_MULTIPLY},
    {"add", VK_ADD},
    {"separator", VK_SEPARATOR},
    {"subtract", VK_SUBTRACT},
    {"decimal", VK_DECIMAL},
    {"divide", VK_DIVIDE},
    {"*", VK_MULTIPLY}, // Duplicates for common symbols
    {"+", VK_ADD},
    {"-", VK_SUBTRACT},
    {".", VK_DECIMAL},
    {"/", VK_DIVIDE},
    {"f1", VK_F1},
    {"f2", VK_F2},
    {"f3", VK_F3},
    {"f4", VK_F4},
    {"f5", VK_F5},
    {"f6", VK_F6},
    {"f7", VK_F7},
    {"f8", VK_F8},
    {"f9", VK_F9},
    {"f10", VK_F10},
    {"f11", VK_F11},
    {"f12", VK_F12},
    {"f13", VK_F13},
    {"f14", VK_F14},
    {"f15", VK_F15},
    {"f16", VK_F16},
    {"f17", VK_F17},
    {"f18", VK_F18},
    {"f19", VK_F19},
    {"f20", VK_F20},
    {"f21", VK_F21},
    {"f22", VK_F22},
    {"f23", VK_F23},
    {"f24", VK_F24},
    {"numlock", VK_NUMLOCK},
    {"scroll", VK_SCROLL},
    {"scrolllock", VK_SCROLL},
    {"lshift", VK_LSHIFT},
    {"rshift", VK_RSHIFT},
    {"lcontrol", VK_LCONTROL},
    {"rcontrol", VK_RCONTROL},
    {"lmenu", VK_LMENU},
    {"rmenu", VK_RMENU},
    {"lalt", VK_LMENU},
    {"ralt", VK_RMENU},
    {"gamepad_a", VK_GAMEPAD_A},
    {"gamepad_b", VK_GAMEPAD_B},
    {"gamepad_x", VK_GAMEPAD_X},
    {"gamepad_y", VK_GAMEPAD_Y},
    {"gamepad_rightshoulder", VK_GAMEPAD_RIGHT_SHOULDER},
    {"gamepad_leftshoulder", VK_GAMEPAD_LEFT_SHOULDER},
    {"gamepad_rightbumper", VK_GAMEPAD_RIGHT_SHOULDER}, // Duplicates for "bumper" instead of "shoulder"
    {"gamepad_leftbumper", VK_GAMEPAD_LEFT_SHOULDER},
    {"gamepad_lefttrigger", VK_GAMEPAD_LEFT_TRIGGER},
    {"gamepad_righttrigger", VK_GAMEPAD_RIGHT_TRIGGER},
    {"gamepad_dpad_up", VK_GAMEPAD_DPAD_UP},
    {"gamepad_dpad_down", VK_GAMEPAD_DPAD_DOWN},
    {"gamepad_dpad_left", VK_GAMEPAD_DPAD_LEFT},
    {"gamepad_dpad_right", VK_GAMEPAD_DPAD_RIGHT},
    {"gamepad_menu", VK_GAMEPAD_MENU},
    {"gamepad_view", VK_GAMEPAD_VIEW},
    {"gamepad_leftthumbstick_button", VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON},
    {"gamepad_rightthumbstick_button", VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON},
    {"gamepad_leftthumbstick_up", VK_GAMEPAD_LEFT_THUMBSTICK_UP},
    {"gamepad_leftthumbstick_down", VK_GAMEPAD_LEFT_THUMBSTICK_DOWN},
    {"gamepad_leftthumbstick_right", VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT},
    {"gamepad_leftthumbstick_left", VK_GAMEPAD_LEFT_THUMBSTICK_LEFT},
    {"gamepad_rightthumbstick_up", VK_GAMEPAD_RIGHT_THUMBSTICK_UP},
    {"gamepad_rightthumbstick_down", VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN},
    {"gamepad_rightthumbstick_right", VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT},
    {"gamepad_rightthumbstick_left", VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT},
    {"gamepad_rightshoulder", VK_GAMEPAD_RIGHT_SHOULDER},
    {"gamepad_leftshoulder", VK_GAMEPAD_LEFT_SHOULDER},
    {"gamepad_rb", VK_GAMEPAD_RIGHT_SHOULDER}, // Duplicates for common shorthand
    {"gamepad_lb", VK_GAMEPAD_LEFT_SHOULDER},
    {"gamepad_lt", VK_GAMEPAD_LEFT_TRIGGER},
    {"gamepad_rt", VK_GAMEPAD_RIGHT_TRIGGER},
    {"gamepad_menu", VK_GAMEPAD_MENU},
    {"gamepad_view", VK_GAMEPAD_VIEW},
    {"gamepad_l3", VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON},
    {"gamepad_r3", VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON},
    {"gamepad_lup", VK_GAMEPAD_LEFT_THUMBSTICK_UP},
    {"gamepad_ldown", VK_GAMEPAD_LEFT_THUMBSTICK_DOWN},
    {"gamepad_lright", VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT},
    {"gamepad_lleft", VK_GAMEPAD_LEFT_THUMBSTICK_LEFT},
    {"gamepad_rup", VK_GAMEPAD_RIGHT_THUMBSTICK_UP},
    {"gamepad_rdown", VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN},
    {"gamepad_rright", VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT},
    {"gamepad_rleft", VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT}
};
std::vector<std::string> virtualKeyNames = [] {
    std::vector<std::string> names(256);
    for (const auto &[name, code]: virtualKeyMap) {
        if (code < names.size() && names[code].empty())
            names[code] = name;
    }
    return names;
}();

void CreateKeybindConfig() {
    std::filesystem::create_directories(keybind_config_dir);
    std::ofstream config(keybind_config_path);
    if (!config.is_open()) {
        LogMessage("Failed to create keybind configs");
        return;
    }

    config << "# Keybind configuration values based on Virtual-Key Codes" << std::endl;
    config << "# https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes" << std::endl << std::endl;
    config << "# Keys may also be in a human-readable format, e.g. F1, Space, Left, etc. Case insensitive." << std::endl;
    config << R"(# Keys may be directly copied from the "Constant" column in the Virtual-Key Codes table in the above documentation,)" << std::endl;
    config << "# with or without the \"VK_\" prefix." << std::endl;
    config << "# Some keys allow shorthand, e.g. N0 for Numpad0, * for Multiply, etc." << std::endl << std::endl;

    std::streamsize maxKeyLength = 0;
    for (const auto &kb: keybinds) {
        if (const auto len = std::strlen(kb.name); len > maxKeyLength) {
            maxKeyLength = static_cast<std::streamsize>(len);
        }
    }

    for (const auto &kb: keybinds) {
        config << std::left << std::setfill(' ') << std::setw(maxKeyLength) << kb.name << " = ";
        config << virtualKeyNames[kb.keycode];
        config << std::endl;
    }

    config.close();

    LogMessage("Created default keybind configs");
}

// Read string from file and interpret as a virtual key code, write to parameter and return true if valid
// Either check for hex value, or string that when lowercased maps to a virtual key code in virtualKeyMap
bool GetVKeyCode(const std::string &str, unsigned char &out) {
    try {
        // Try hex value first
        out = static_cast<unsigned char>(std::stoul(str, nullptr, 0));
        return true;
    } catch (const std::exception &) {
        if (const auto it = virtualKeyMap.find(str); it != virtualKeyMap.end()) {
            out = it->second;
            return true;
        }
    }

    return false;
}

void Trim(std::string &str) {
    const size_t start = str.find_first_not_of(' ');
    const size_t end = str.find_last_not_of(' ');

    if (start == std::string::npos)
        str.clear();  // String contains only whitespace
    else
        str = str.substr(start, end - start + 1);
}

void ReadKeybindConfig() {
    std::ifstream config(keybind_config_path);
    if (!config.is_open()) {
        LogMessage("No keybinds.txt found, creating defaults.");
        CreateKeybindConfig();
        return;
    }

    std::string line;
    while (std::getline(config, line)) {
        if (line.empty() || line[0] == '#')
            continue;

        // Remove comments at end of line
        if (line.find('#') != std::string::npos)
            line = line.substr(0, line.find('#'));

        // Expected format: Action=KEYCODE
        // Example: ToggleNoClip=0x90
        const size_t delimPos = line.find('=');
        if (delimPos == std::string::npos)
            continue;

        std::string key = line.substr(0, delimPos);
        std::string value = line.substr(delimPos + 1);

        // Trim key/value and lowercase value
        Trim(key);
        Trim(value);
        std::transform(value.begin(), value.end(), value.begin(), [](const unsigned char c) { return std::tolower(c); });
        if (value.find("vk_") == 0) // Allow optional "VK_" prefix
            value = value.substr(3);

        unsigned char keycode;
        if (!GetVKeyCode(value, keycode)) {
            printf("Invalid keycode %s for action %s in config, skipping\n", value.c_str(), key.c_str());
            continue;
        }

        // Map action to keycode
        for (auto &kb: keybinds) {
            if (key == kb.name) {
                printf("Mapped action %s to keycode 0x%02X from %s\n", key.c_str(), keycode, value.c_str());
                kb.keycode = keycode;
                break;
            }
        }
    }
}

bool GetKey(const keybind &kb) {
    const auto vkCode = kb.keycode;
    if (kb.hold)
        return current[vkCode] & 0xF0;
    return (current[vkCode] & 0xF0) && !(previous[vkCode] & 0xF0);
}

bool TryActivate(const byte kCode) {
    if (const auto &kb = keybinds[kCode]; GetKey(kb)) {
        kb.callback();
        return true;
    }
    return false;
}
