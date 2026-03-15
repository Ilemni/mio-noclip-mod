#include "mio-modding-api.h"
#include "noclip.h"

#include <string>
#include "Windows.h"

char* modId;
void LogMessage(const char* message) {
    ModAPI::Util::LogMessage(modId, message);
}

extern "C" __declspec(dllexport) void ModInit(const char* id) {
    modId = _strdup(id);
    CreateThread(nullptr, 0, NoClip, nullptr, 0, nullptr);
}

auto APIENTRY DllMain(HMODULE, DWORD, LPVOID) -> BOOL {
    return TRUE;
}
