#include <Windows.h>
#include <string>
#include <vector>


class MemoryUtility {
public:
    static DWORD_PTR modBaseAddress;
    static DWORD_PTR tmodBaseAddress;

    static DWORD_PTR GetModuleBaseAddress(DWORD processId, const std::wstring& moduleName);

    static int ReadInt32(DWORD_PTR address);
    static long long ReadInt64(DWORD_PTR address);
    static std::string ReadString(DWORD_PTR address, int length);
    static DWORD_PTR GetPtrAddr(DWORD_PTR base, const std::vector<DWORD>& offsets);

    static bool IsValidAddress(DWORD_PTR address);
    static double ReadVideoTime();
    static std::vector<std::string> ReadVideoId();
    
    static double ReadDouble(DWORD_PTR address);

    static void initialize();
};