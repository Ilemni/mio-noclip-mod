#include <windows.h>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iosfwd>
#include <string>
#include <vector>

#include "modding_api.h"

#define K_TOGGLENOCLIP 0
#define K_PRINTPOS 1
#define K_MOVELEFT 2
#define K_MOVERIGHT 3
#define K_MOVEDOWN 4
#define K_MOVEUP 5
#define K_MOVEBACKWARD 6
#define K_MOVEFORWARD 7
#define K_RESETZ 8
#define K_DECREASESPEED 9
#define K_INCREASESPEED 10
#define K_SAVEPOS 11
#define K_LOADPOS 12

const auto keybind_config_dir = "modconfig/noclip/";
const auto keybind_config_path = "modconfig/noclip/keybinds.txt";

// Program vars
bool g_Running = true;
HWND g_HWND = nullptr;

// Input
std::vector<unsigned char> previous(256);
std::vector<unsigned char> current(256);

// Keybinds
struct keybind {
    const char *name{};
    unsigned char keycode{};
    bool hold{};
    std::function<void(f32x3 *)> callback;
};

std::vector<keybind> keybinds(13);

// Gameplay vars
f32x3 savedLoc = make_f32x3(0, 0, 0);
f32x3 locLastFrame = make_f32x3(0, 0, 0);
float speed = 0.0625f;
int hpLastFrame;
int hpOnEnterNoClip;
bool noClipEnabled;

bool GetKey(const keybind &kb) {
    const auto vkCode = kb.keycode;
    if (kb.hold)
        return current[vkCode] & 0xF0;
    return (current[vkCode] & 0xF0) && !(previous[vkCode] & 0xF0);
}

bool GetVKeyDown(byte vkCode) {
    return (current[vkCode] & 0xF0) && !(previous[vkCode] & 0xF0);
}

void SetLoc(f32x3 loc) {
    SetPlayerLocation(loc);
    locLastFrame = loc;
}

bool TryActivate(byte kCode, f32x3 *loc) {
    if (const auto &kb = keybinds[kCode]; GetKey(kb)) {
        kb.callback(loc);
        return true;
    }
    return false;
}

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

void LoadKeybinds() {
    const auto toggleNoClip = [](const f32x3 *loc) {
        noClipEnabled = !noClipEnabled;
        locLastFrame = *loc;
        if (noClipEnabled) {
            hpOnEnterNoClip = GetPlayerHealth();
            hpLastFrame = max(1, hpOnEnterNoClip);
        } else {
            SetPlayerHealth(hpOnEnterNoClip);
            printf("Set HP to %d,", hpOnEnterNoClip);
        }
    };

    const auto printLoc = [](f32x3 *loc) {
        std::printf("Player location: %.2f, %.2f, %.2f\n", loc->x, loc->y, loc->z);
    };

    const auto moveLeft = [](f32x3 *loc) { if (noClipEnabled) loc->x -= speed; };
    const auto moveRight = [](f32x3 *loc) { if (noClipEnabled) loc->x += speed; };
    const auto moveDown = [](f32x3 *loc) { if (noClipEnabled) loc->y -= speed; };
    const auto moveUp = [](f32x3 *loc) { if (noClipEnabled) loc->y += speed; };
    const auto moveForward = [](f32x3 *loc) { if (noClipEnabled) loc->z += speed; };
    const auto moveBackward = [](f32x3 *loc) { if (noClipEnabled) loc->z -= speed; };
    const auto resetZ = [](f32x3 *loc) { if (noClipEnabled) loc->z = 0; };
    const auto decreaseSpeed = [](f32x3 *) { speed *= 0.5f; };
    const auto increaseSpeed = [](f32x3 *) { speed *= 2.0f; };
    const auto savePosition = [](const f32x3 *loc) { savedLoc = *loc; };
    const auto loadPosition = [](f32x3 *) { if (noClipEnabled) SetLoc(savedLoc); };

    keybinds[K_TOGGLENOCLIP] = {"ToggleNoClip", VK_NUMPAD2, false, toggleNoClip};
    keybinds[K_PRINTPOS] = {"PrintPosition", VK_ADD, false, printLoc};
    keybinds[K_MOVELEFT] = {"MoveLeft", VK_NUMPAD4, true, moveLeft};
    keybinds[K_MOVERIGHT] = {"MoveRight", VK_NUMPAD6, true, moveRight};
    keybinds[K_MOVEDOWN] = {"MoveDown", VK_NUMPAD5, true, moveDown};
    keybinds[K_MOVEUP] = {"MoveUp", VK_NUMPAD8, true, moveUp};
    keybinds[K_MOVEBACKWARD] = {"MoveBackward", VK_NUMPAD7, true, moveBackward};
    keybinds[K_MOVEFORWARD] = {"MoveForward", VK_NUMPAD9, true, moveForward};
    keybinds[K_RESETZ] = {"ResetZ", VK_NUMPAD0, false, resetZ};
    keybinds[K_DECREASESPEED] = {"DecreaseSpeed", VK_NUMPAD1, false, decreaseSpeed};
    keybinds[K_INCREASESPEED] = {"IncreaseSpeed", VK_NUMPAD3, false, increaseSpeed};
    keybinds[K_SAVEPOS] = {"SavePosition", VK_DIVIDE, false, savePosition};
    keybinds[K_LOADPOS] = {"LoadPosition", VK_MULTIPLY, false, loadPosition};

    ReadKeybindConfig();
}

void UpdateInput() {
    const unsigned long idActive = GetWindowThreadProcessId(g_HWND, nullptr);
    if (AttachThreadInput(GetCurrentThreadId(), idActive, TRUE)) {
        GetKeyState(0);
        current.swap(previous);
        GetKeyboardState(&current[0]);

        AttachThreadInput(GetCurrentThreadId(), idActive, FALSE);
    }
}

DWORD WINAPI MyModCode(LPVOID lpParam) {
    LoadKeybinds();

    // Looking for game window
    while (g_Running && !g_HWND) {
        g_HWND = FindWindowW(nullptr, L"MIO");
        if (g_HWND)
            printf("Found game window! 0x%p\n", g_HWND);
        else
            Sleep(100);
    }

    while (true) {
        if (!g_HWND)
            return 0;

        Sleep(1); // Update every 1ms, 10ms is a bit too jittery
        UpdateInput();

        f32x3 loc = GetPlayerLocation(); // Gets the player's location
        // Debug print location
        if (GetVKeyDown(VK_DELETE)) {
            LogMessage("Reloading keybind config...");
            ReadKeybindConfig();
        }

        // Debug print location
        TryActivate(K_PRINTPOS, &loc);

        // Cancel noclip if not a valid location (e.g. main menu or credits)
        if (loc.x == -1 && loc.y == -1 && loc.z == -1) {
            savedLoc = loc;
            noClipEnabled = false;
            Sleep(10);
            continue;
        }

        // Toggle noclip
        TryActivate(K_TOGGLENOCLIP, &loc);

        // "No-clip" until we figure out preventing damage
        if (noClipEnabled) {
            const int hp = max(1, hpLastFrame);
            SetPlayerHealth(hp);
            SetPlayerLocation(locLastFrame);
            loc = locLastFrame;
        }

        // Save/Set location
        TryActivate(K_SAVEPOS, &loc) || TryActivate(K_LOADPOS, &loc);

        // Speed modifiers
        TryActivate(K_DECREASESPEED, &loc) || TryActivate(K_INCREASESPEED, &loc);


        if (noClipEnabled) {
            TryActivate(K_MOVELEFT, &loc);
            TryActivate(K_MOVERIGHT, &loc);
            TryActivate(K_MOVEDOWN, &loc);
            TryActivate(K_MOVEUP, &loc);
            TryActivate(K_MOVEBACKWARD, &loc);
            TryActivate(K_MOVEFORWARD, &loc);
            TryActivate(K_RESETZ, &loc);

            SetLoc(loc);
        }

        hpLastFrame = GetPlayerHealth();
    }
    return 0;
}

extern "C" __declspec(dllexport) void ModInit() {
    LogMessage("\"Noclip\" mod loaded!");

    // Start monitoring thread
    CreateThread(nullptr, 0, MyModCode, nullptr, 0, nullptr);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_DETACH)
        g_Running = false;

    return TRUE;
}
