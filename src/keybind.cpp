#include "keybind.h"

#include "modding_api.h"

#include <filesystem>
#include <fstream>
#include <string>

const auto keybind_config_dir = "modconfig/noclip/";
const auto keybind_config_path = "modconfig/noclip/keybinds.txt";

void CreateKeybindConfig() {
    std::filesystem::create_directories(keybind_config_dir);
    std::ofstream config(keybind_config_path);
    if (!config.is_open()) {
        LogMessage("Failed to create keybind configs");
        return;
    }

    config << "# Keybind configuration values based on\n";
    config << "# Virtual-Key Codes\n";
    config << "# https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes\n\n";

    for (const auto &kb: keybinds) {
        config << kb.name << "=0x";
        config << std::hex << std::noshowbase << std::uppercase << static_cast<int>(kb.keycode);
        config << std::endl;
    }

    config.close();

    LogMessage("Created default keybind configs");
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

        std::string action = line.substr(0, delimPos);
        const unsigned char keycode = static_cast<unsigned char>(std::stoul(line.substr(delimPos + 1), nullptr, 0));

        // Map action to keycode
        for (auto &kb: keybinds) {
            if (action == kb.name) {
                printf("Mapped action %s to keycode 0x%02X\n", action.c_str(), keycode);
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

bool TryActivate(byte kCode) {
    if (const auto &kb = keybinds[kCode]; GetKey(kb)) {
        kb.callback();
        return true;
    }
    return false;
}
