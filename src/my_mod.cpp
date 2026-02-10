#include <windows.h>
#include <stdio.h>
#include "modding_api.h"

DWORD WINAPI MyModCode(LPVOID lpParam) {
    // Code from "Slight Breeze" mod
    while (true) {
        Sleep(10);  // Update every 10ms
        
        f32x3 loc = GetPlayerLocation(); // Gets the player's location
        
        if (loc.x == -1.0f || loc.y == -1.0f) {
            continue; // Wait until save is loaded
        }
        printf("Old Player Position: X=%.2f, Y=%.2f, Z=%.2f     \n", loc.x, loc.y, loc.z);

        loc.x += 0.03;
        SetPlayerLocation(loc); // Sets the player's location to the new location
        
        printf("New Player Position: X=%.2f, Y=%.2f, Z=%.2f     \n", loc.x, loc.y, loc.z);
        fflush(stdout);
    }
    return 0;
}

extern "C" __declspec(dllexport) void ModInit() {
    LogMessage("\"My\" Mod mod loaded!");
    
    // Start monitoring thread
    CreateThread(nullptr, 0, MyModCode, nullptr, 0, nullptr);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    // Mods don't need to do anything in DllMain
    return TRUE;
}