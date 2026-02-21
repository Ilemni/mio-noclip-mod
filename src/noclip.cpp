#include "noclip.h"

#include "modding_api.h"
#include "my_mod.h"
#include "keybind.h"
#include "teleport_list.h"

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
#define K_RELOADCONFIG 13
#define K_PREVTELEPORT 14
#define K_NEXTTELEPORT 15

// Gameplay vars
f32x3 savedLoc = make_f32x3(0, 0, 0);
f32x3 locLastFrame = make_f32x3(0, 0, 0);
f32x3 currentLoc = make_f32x3(0, 0, 0);

float speed = 0.0625f;
int hpLastFrame;
int hpOnEnterNoClip;
bool noClipEnabled;

void SetLoc(const f32x3 loc) {
    SetPlayerLocation(loc);
    locLastFrame = loc;
}

void LoadKeybinds() {
    const auto toggleNoClip = [] {
        noClipEnabled = !noClipEnabled;
        locLastFrame = currentLoc;
        if (noClipEnabled) {
            hpOnEnterNoClip = GetPlayerHealth();
            hpLastFrame = max(1, hpOnEnterNoClip);
        } else {
            SetPlayerHealth(hpOnEnterNoClip);
            printf("Set HP to %d,", hpOnEnterNoClip);
        }
    };

    const auto printLoc = [] {
        std::printf("Player location: %.2f, %.2f, %.2f\n%.2f %.2f (for copying to locations.txt)\n",
            currentLoc.x, currentLoc.y, currentLoc.z, currentLoc.x, currentLoc.y);
    };

    const auto moveLeft = [] { if (noClipEnabled) currentLoc.x -= speed; };
    const auto moveRight = [] { if (noClipEnabled) currentLoc.x += speed; };
    const auto moveDown = [] { if (noClipEnabled) currentLoc.y -= speed; };
    const auto moveUp = [] { if (noClipEnabled) currentLoc.y += speed; };
    const auto moveForward = [] { if (noClipEnabled) currentLoc.z += speed; };
    const auto moveBackward = [] { if (noClipEnabled) currentLoc.z -= speed; };
    const auto resetZ = [] { if (noClipEnabled) currentLoc.z = 0; };
    const auto decreaseSpeed = [] { speed *= 0.5f; };
    const auto increaseSpeed = [] { speed *= 2.0f; };
    const auto savePosition = [] { savedLoc = currentLoc; };
    const auto loadPosition = [] { if (noClipEnabled) SetLoc(savedLoc); };
    const auto reloadConfig = [] { ReadKeybindConfig(); ReadLocations(); };
    const auto prevTeleport = [] { PrevTeleport(&currentLoc); };
    const auto nextTeleport = [] { NextTeleport(&currentLoc); };

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
    keybinds[K_RELOADCONFIG] = {"ReloadConfig", VK_DELETE, false, reloadConfig};
    keybinds[K_PREVTELEPORT] = {"PrevTeleport", VK_SUBTRACT, false, prevTeleport };
    keybinds[K_NEXTTELEPORT] = {"NextTeleport", VK_ADD, false, nextTeleport};

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

DWORD WINAPI NoClip(LPVOID lpParam) {
    LoadKeybinds();
    ReadLocations();

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

        currentLoc = GetPlayerLocation(); // Gets the player's location

        TryActivate(K_RELOADCONFIG);

        // Debug print location
        TryActivate(K_PRINTPOS);

        // Cancel noclip if not a valid location (e.g. main menu or credits)
        if (currentLoc.x == -1 && currentLoc.y == -1 && currentLoc.z == -1) {
            savedLoc = currentLoc;
            noClipEnabled = false;
            Sleep(10);
            continue;
        }

        // Toggle noclip
        TryActivate(K_TOGGLENOCLIP);

        // "No-clip" until we figure out preventing damage
        if (noClipEnabled) {
            const int hp = max(1, hpLastFrame);
            SetPlayerHealth(hp);
            SetPlayerLocation(locLastFrame);
            currentLoc = locLastFrame;
        }

        // Save/Set location
        TryActivate(K_SAVEPOS) || TryActivate(K_LOADPOS);

        // Speed modifiers
        TryActivate(K_DECREASESPEED) || TryActivate(K_INCREASESPEED);


        if (noClipEnabled) {
            TryActivate(K_PREVTELEPORT) || TryActivate(K_NEXTTELEPORT);
            TryActivate(K_MOVELEFT);
            TryActivate(K_MOVERIGHT);
            TryActivate(K_MOVEDOWN);
            TryActivate(K_MOVEUP);
            TryActivate(K_MOVEBACKWARD);
            TryActivate(K_MOVEFORWARD);
            TryActivate(K_RESETZ);

            SetLoc(currentLoc);
        }

        hpLastFrame = GetPlayerHealth();
    }
    return 0;
}
