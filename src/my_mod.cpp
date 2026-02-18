#include <windows.h>
#include <cstdio>
#include <vector>

#include "modding_api.h"

// Program vars
bool g_Running = true;
HWND g_HWND = nullptr;

// Input
std::vector<unsigned char> previous(256);
std::vector<unsigned char> current(256);

// Gameplay vars
f32x3 savedLoc = make_f32x3(0, 0, 0);
f32x3 locLastFrame = make_f32x3(0, 0, 0);
float speed = 0.0625f;
int hpLastFrame;
bool noClipEnabled;

// As someone that mainly writes C#... where's my expression-bodied members :(
bool GetKeyDown(byte vkCode) { return (current[vkCode] & 0xF0) && !(previous[vkCode] & 0xF0); }

bool GetKeyUp(byte vkCode) { return !(current[vkCode] & 0xF0) && (previous[vkCode] & 0xF0); }

bool GetKey(byte vkCode) { return current[vkCode] & 0xF0; }

void SetLoc(f32x3 loc) {
    SetPlayerLocation(loc);
    locLastFrame = loc;
}

void UpdateInput() {
    if (!g_HWND)
        return;

    unsigned long idActive = GetWindowThreadProcessId(g_HWND, nullptr);

    if (AttachThreadInput(GetCurrentThreadId(), idActive, TRUE))
    {
        GetKeyState(0);
        current.swap(previous);
        GetKeyboardState(&current[0]);

        AttachThreadInput(GetCurrentThreadId(), idActive, FALSE);
    }
}

DWORD WINAPI MyModCode(LPVOID lpParam) {
    // Looking for game window
    while (g_Running && !g_HWND)
    {
        g_HWND = FindWindowW(nullptr, L"MIO");
        if (g_HWND)
            printf("Found game window! 0x%p\n", g_HWND);
        else
            Sleep(100);
    }

    while (true) {
        Sleep(1);  // Update every 1ms, 10ms is a bit too jittery
        UpdateInput();

        f32x3 loc = GetPlayerLocation(); // Gets the player's location

        // Debug print location
        if (GetKeyDown(VK_ADD)) {
            std::printf("Player location: %.2f, %.2f, %.2f\n", loc.x, loc.y, loc.z);
        }

        // Cancel noclip if not a valid location (e.g. main menu or credits)
        if (loc.x == -1 && loc.y == -1 && loc.z == -1) {
            savedLoc = loc;
            noClipEnabled = false;
            Sleep(10);
            continue;
        }

        // "No-clip" until we figure out preventing damage
        if (noClipEnabled) {
            const int hp = min(1, hpLastFrame);
            SetPlayerHealth(hp);
            SetPlayerLocation(locLastFrame);
            loc = locLastFrame;
        }

        // Toggle noclip
        if (GetKeyDown(VK_NUMPAD2)) {
            noClipEnabled = !noClipEnabled;
            locLastFrame = loc;
            hpLastFrame = GetPlayerHealth();
        }

        // Save/Set location
        if (GetKeyDown(VK_DIVIDE))
            savedLoc = loc;
        else if (GetKeyDown(VK_MULTIPLY) && noClipEnabled)
            SetLoc(savedLoc);

        // Speed modifiers
        if (GetKeyDown(VK_NUMPAD1))
            speed *= 0.5f;
        else if (GetKeyDown(VK_NUMPAD3))
            speed *= 2.0f;

        if (noClipEnabled) {
            // Left/Right (X axis)
            if (GetKey(VK_NUMPAD4))
                loc.x -= speed;
            if (GetKey(VK_NUMPAD6))
                loc.x += speed;

            // Up/Down (Y axis)
            if (GetKey(VK_NUMPAD5))
                loc.y -= speed;
            if (GetKey(VK_NUMPAD8))
                loc.y += speed;

            // Forward/Backward (Z axis)
            if (GetKey(VK_NUMPAD7))
                loc.z -= speed;
            if (GetKey(VK_NUMPAD9))
                loc.z += speed;
            // Reset Z
            if (GetKeyDown(VK_NUMPAD0))
                loc.z = 0;

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