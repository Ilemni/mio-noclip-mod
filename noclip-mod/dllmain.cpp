#include "mio-modding-api.h"
#include "noclip.h"

#include "Windows.h"

char* modId;

void LogMessage(const char* message) {
    ModAPI::Util::LogMessage(modId, message);
}

extern "C" __declspec(dllexport) void ModInit(char* id) {
    modId = id;
    CreateThread(nullptr, 0, NoClip, nullptr, 0, nullptr);
}

auto APIENTRY DllMain(HMODULE, DWORD, LPVOID) -> BOOL {
    return TRUE;
}
