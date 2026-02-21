#include "my_mod.h"

#include "modding_api.h"
#include "noclip.h"

extern "C" __declspec(dllexport) void ModInit() {
    LogMessage("\"Noclip\" mod loaded!");

    // Start monitoring thread
    CreateThread(nullptr, 0, NoClip, nullptr, 0, nullptr);
}

BOOL APIENTRY DllMain(HMODULE, DWORD ul_reason_for_call, LPVOID) {
    if (ul_reason_for_call == DLL_PROCESS_DETACH)
        g_Running = false;

    return TRUE;
}
