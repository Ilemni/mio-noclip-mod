#include "noclip.h"
#include "mio-modding-api.h"
#include "dllmain.h"
#include "keybind.h"

#include <algorithm>

using ModAPI::Player::GetPlayerLocation;
using ModAPI::Player::SetPlayerLocation;

namespace {
    namespace Input {
        keybind* toggleNoClip = &keybinds[0];
        keybind* moveLeft = &keybinds[1];
        keybind* moveRight = &keybinds[2];
        keybind* moveDown = &keybinds[3];
        keybind* moveUp = &keybinds[4];
        keybind* moveBackward = &keybinds[5];
        keybind* moveForward = &keybinds[6];
        keybind* resetZ = &keybinds[7];
        keybind* decreaseSpeed = &keybinds[8];
        keybind* increaseSpeed = &keybinds[9];
        keybind* reloadConfig = &keybinds[10];
        keybind* printPosition = &keybinds[11];
    }

    HWND mioHwnd = nullptr;
    uintptr_t baseAddr;

    // Gameplay vars
    auto locLastFrame = Vector3(0, 0, 0);
    auto currentLoc = Vector3(0, 0, 0);

    float speed = 0.0625f;
    bool isNoClip = false;

    void LoadKeybinds() {
        *Input::toggleNoClip = {.name = "ToggleNoClip", .keycode = VK_NUMPAD2, .hold = false};
        *Input::printPosition = {.name = "PrintPosition", .keycode = VK_ADD, .hold = false};
        *Input::moveLeft = {.name = "MoveLeft", .keycode = VK_NUMPAD4, .hold = true};
        *Input::moveRight = {.name = "MoveRight", .keycode = VK_NUMPAD6, .hold = true};
        *Input::moveDown = {.name = "MoveDown", .keycode = VK_NUMPAD5, .hold = true};
        *Input::moveUp = {.name = "MoveUp", .keycode = VK_NUMPAD8, .hold = true};
        *Input::moveBackward = {.name = "MoveBackward", .keycode = VK_NUMPAD7, .hold = true};
        *Input::moveForward = {.name = "MoveForward", .keycode = VK_NUMPAD9, .hold = true};
        *Input::resetZ = {.name = "ResetZ", .keycode = VK_NUMPAD0, .hold = false};
        *Input::decreaseSpeed = {.name = "DecreaseSpeed", .keycode = VK_NUMPAD1, .hold = false};
        *Input::increaseSpeed = {.name = "IncreaseSpeed", .keycode = VK_NUMPAD3, .hold = false};
        *Input::reloadConfig = {.name = "ReloadConfig", .keycode = VK_DELETE, .hold = false};

        ReadKeybindConfig();
    }

    void UpdateInput() {
        if (GetForegroundWindow() != mioHwnd) {
            // Clear input when not focused
            std::ranges::fill(current, 0);
            current.swap(previous);
            return;
        }

        const unsigned long idActive = GetWindowThreadProcessId(mioHwnd, nullptr);
        if (AttachThreadInput(GetCurrentThreadId(), idActive, TRUE)) {
            GetKeyState(0);
            current.swap(previous);
            GetKeyboardState(current.data());

            AttachThreadInput(GetCurrentThreadId(), idActive, FALSE);
        }
    }
}

DWORD WINAPI NoClip(LPVOID) {
    HMODULE module = GetModuleHandleA("mio.exe");
    if (!module) {
        LogMessage("ERROR: Failed to get mio.exe module handle!");
        return 0;
    }
    baseAddr = reinterpret_cast<uintptr_t>(module);

    LoadKeybinds();

    // Looking for game window
    while (!mioHwnd) {
        mioHwnd = FindWindowW(nullptr, L"MIO");
        if (mioHwnd)
            LogMessage("Found game window! {:#16x}", reinterpret_cast<uintptr_t>(mioHwnd));
        else
            Sleep(100);
    }

    while (true) {
        // Sleep(1); // Update every 1ms, 10ms is a bit too jittery
        UpdateInput();

        currentLoc = GetPlayerLocation(); // Gets the player's location

        if (Input::reloadConfig->active()) {
            ReadKeybindConfig();
        }

        // Debug print location
        if (Input::printPosition->active()) {
            LogMessage("Player location: {:.2f}, {:.2f}, {:.2f}", currentLoc.x, currentLoc.y, currentLoc.z);
        }

        // Cancel noclip if not a valid location (e.g. main menu or credits)
        if (currentLoc.x == -1 && currentLoc.y == -1 && currentLoc.z == -1) { // NOLINT(clang-diagnostic-float-equal)
            if (isNoClip) {
                isNoClip = false;
                LogMessage("NoClip disabled (invalid location)");
            }
            Sleep(10);
            continue;
        }

        // Toggle noclip
        if (Input::toggleNoClip->active()) {
            isNoClip = !isNoClip;

            LogMessage("NoClip {}", isNoClip ? "enabled." : "disabled.");
            locLastFrame = currentLoc;
        }

        // "No-clip" until we figure out preventing damage
        if (isNoClip) {
            SetPlayerLocation(locLastFrame);
            currentLoc = locLastFrame;
        }

        // Speed modifiers
        if (Input::decreaseSpeed->active()) {
            speed *= 0.5f;
        }
        if (Input::increaseSpeed->active()) {
            speed *= 2.0f;
        }

        if (!isNoClip) {
            continue;
        }

        float x = 0, y = 0, z = 0;
        if (Input::moveLeft->active()) {
            x -= speed;
        }
        if (Input::moveRight->active()) {
            x += speed;
        }
        if (Input::moveDown->active()) {
            y -= speed;
        }
        if (Input::moveUp->active()) {
            y += speed;
        }
        if (Input::moveBackward->active()) {
            z -= speed;
        }
        if (Input::moveForward->active()) {
            z += speed;
        }
        if (Input::resetZ->active()) {
            z = -currentLoc.z;
        }

        currentLoc.x += x;
        currentLoc.y += y;
        currentLoc.z += z;
        SetPlayerLocation(currentLoc);
        locLastFrame = currentLoc;
    }
}
