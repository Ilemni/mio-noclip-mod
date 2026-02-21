#include <windows.h>
#include <cstdio>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

#include "modding_api.h"
#include "keybind.h"

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

// Program vars
bool g_Running = true;
HWND g_HWND = nullptr;

// Gameplay vars
f32x3 savedLoc = make_f32x3(0, 0, 0);
f32x3 locLastFrame = make_f32x3(0, 0, 0);
float speed = 0.0625f;
int hpLastFrame;
int hpOnEnterNoClip;
bool noClipEnabled;

bool GetVKeyDown(byte vkCode) {
    return (current[vkCode] & 0xF0) && !(previous[vkCode] & 0xF0);
}

void SetLoc(f32x3 loc) {
    SetPlayerLocation(loc);
    locLastFrame = loc;
}

void LoadKeybinds() {
    const auto toggleNoClip = [](f32x3 *loc) {
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
    const auto savePosition = [](f32x3 *loc) { savedLoc = *loc; };
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
    if (GetForegroundWindow() != g_HWND) {
        // Clear input when not focused
        std::fill(current.begin(), current.end(), 0);
        current.swap(previous);
        return;
    }

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
