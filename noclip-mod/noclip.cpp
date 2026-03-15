#include "noclip.h"
#include "mio-modding-api.h"
#include "dllmain.h"
#include "keybind.h"

#include <algorithm>

namespace {
    namespace Addresses {
        void* playerSetXYOpAddr = nullptr;
        void* playerSetZOpAddr1 = nullptr;
        void* playerSetZOpAddr2 = nullptr;
        char* playerSetXYOpOriginalBytes = nullptr;
        char* playerSetZOpOriginalBytes1 = nullptr;
        char* playerSetZOpOriginalBytes2 = nullptr;

        auto LoadMemoryAddresses() -> bool {
            HMODULE h_module = GetModuleHandleA("mio.exe");
            if (!h_module) {
                LogMessage("ERROR: Failed to get mio.exe module handle!");
                return false;
            }

            const uintptr_t base_addr = reinterpret_cast<uintptr_t>(h_module);

            playerSetXYOpAddr = reinterpret_cast<void*>(base_addr + 0x009D9B75); // NOLINT(performance-no-int-to-ptr)
            playerSetZOpAddr1 = reinterpret_cast<void*>(base_addr + 0x009D9B7D); // NOLINT(performance-no-int-to-ptr)
            playerSetZOpAddr2 = reinterpret_cast<void*>(base_addr + 0x009F107E); // NOLINT(performance-no-int-to-ptr)
            playerSetXYOpOriginalBytes = new char[3];
            playerSetZOpOriginalBytes1 = new char[5];
            playerSetZOpOriginalBytes2 = new char[5];
            ModAPI::Util::ReadMemory(playerSetXYOpAddr, playerSetXYOpOriginalBytes, 3);
            ModAPI::Util::ReadMemory(playerSetZOpAddr1, playerSetZOpOriginalBytes1, 5);
            ModAPI::Util::ReadMemory(playerSetZOpAddr2, playerSetZOpOriginalBytes2, 5);
            return true;
        }

    }
    namespace Pointers {
        void* playerCollisionTypeBasePtr = nullptr;

        auto LoadMemoryAddresses() -> bool {
            HMODULE h_module = GetModuleHandleA("mio.exe");
            if (!h_module) {
                LogMessage("ERROR: Failed to get mio.exe module handle!");
                return false;
            }

            const uintptr_t base_addr = reinterpret_cast<uintptr_t>(h_module);

            playerCollisionTypeBasePtr = reinterpret_cast<void*>(base_addr + 0x010EFF48); // NOLINT(performance-no-int-to-ptr)
            return true;
        }
    }

    namespace Player {
        using ModAPI::Util::FollowPointer;

        byte GetPlayerCollisionType() {
            byte result = 0;
            if (!Pointers::playerCollisionTypeBasePtr) {
                return result;
            }

            void* collisionTypeAddr = FollowPointer(Pointers::playerCollisionTypeBasePtr, 0x14);
            if (collisionTypeAddr) {
                result = ModAPI::Util::ReadMemoryTyped<byte>(collisionTypeAddr);
            }

            return result;
        }

        bool SetPlayerCollisionType(const byte type) {
            if (!Pointers::playerCollisionTypeBasePtr) {
                return false;
            }

            void* collisionTypeAddr = FollowPointer(Pointers::playerCollisionTypeBasePtr, 0x14);
            bool success = false;
            if (collisionTypeAddr) {
                success = ModAPI::Util::WriteMemoryTyped(collisionTypeAddr, type);
            }

            return success;
        }

        void DisableVanillaPlayerMovement() {
            // Disable vanilla player Z movement by NOPing the instructions that set the player Z position each frame, we'll handle it ourselves in the mod tick
            ModAPI::Util::NopBytes(Addresses::playerSetXYOpAddr, 3);
            ModAPI::Util::NopBytes(Addresses::playerSetZOpAddr1, 5);
            ModAPI::Util::NopBytes(Addresses::playerSetZOpAddr2, 5);
        }

        void RestoreVanillaPlayerMovement() {
            ModAPI::Util::WriteMemory(Addresses::playerSetXYOpAddr, Addresses::playerSetXYOpOriginalBytes, 3);
            ModAPI::Util::WriteMemory(Addresses::playerSetZOpAddr1, Addresses::playerSetZOpOriginalBytes1, 5);
            ModAPI::Util::WriteMemory(Addresses::playerSetZOpAddr2, Addresses::playerSetZOpOriginalBytes2, 5);
        }
    }

    namespace Input {
        keybind* toggleNoClip = &keybinds[0];
        keybind* moveUp = &keybinds[1];
        keybind* moveLeft = &keybinds[2];
        keybind* moveDown = &keybinds[3];
        keybind* moveRight = &keybinds[4];
        keybind* moveBackward = &keybinds[5];
        keybind* moveForward = &keybinds[6];
        keybind* decreaseSpeed = &keybinds[7];
        keybind* increaseSpeed = &keybinds[8];
        keybind* resetZ = &keybinds[9];
        keybind* reloadConfig = &keybinds[10];
        keybind* printPosition = &keybinds[11];
    }

    HWND mioHwnd = nullptr;
    uintptr_t baseAddr;

    // Gameplay vars
    auto locLastFrame = Vector3(0, 0, 0);
    auto currentLoc = Vector3(0, 0, 0);

    float speed = 0.0625f;
    byte prevCollisionType = 0;
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
    Addresses::LoadMemoryAddresses();
    Pointers::LoadMemoryAddresses();
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
        Sleep(1); // Update every 1ms, 10ms is a bit too jittery
        UpdateInput();

        currentLoc = ModAPI::Player::GetPlayerLocation(); // Gets the player's location

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
                Player::RestoreVanillaPlayerMovement();
                LogMessage("NoClip disabled (invalid location)");
            }
            Sleep(10);
            continue;
        }

        // Toggle noclip
        if (Input::toggleNoClip->active()) {
            isNoClip = !isNoClip;
            if (isNoClip) {
                prevCollisionType = Player::GetPlayerCollisionType();
                Player::SetPlayerCollisionType(255);
                Player::DisableVanillaPlayerMovement();
            }
            else {
                Player::SetPlayerCollisionType(prevCollisionType);
                Player::RestoreVanillaPlayerMovement();
            }

            LogMessage("NoClip {}", isNoClip ? "enabled." : "disabled.");
            locLastFrame = currentLoc;
        }

        // "No-clip" until we figure out preventing damage
        if (isNoClip) {
            ModAPI::Player::SetPlayerLocation(locLastFrame);
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
        ModAPI::Player::SetPlayerLocation(currentLoc);
        locLastFrame = currentLoc;
    }
}
