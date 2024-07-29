#include <windows.h>

// outsmart GCC's missing-declarations warning
BOOL WINAPI DiscordDllMain(HMODULE, DWORD, LPVOID);
BOOL WINAPI DiscordDllMain(HMODULE, DWORD, LPVOID)
{
    return TRUE;
}
