#include "MemoryUtility.h"
#include <vector>
#include <thread>
#include <chrono>
#include <tlhelp32.h>
#include "fstream"
#include <iostream>
#include <curl/curl.h>
//#include <Utils.h>


DWORD_PTR MemoryUtility::modBaseAddress = 0;
DWORD_PTR MemoryUtility::tmodBaseAddress = 0;


//using namespace Utils;

// Function to read an int from memory
int MemoryUtility::ReadInt32(DWORD_PTR address) {
    if (address == 0) {
        return static_cast<int>(0);
    }

    int value = static_cast<int>(0);
    __try {
        value = *(int*)address;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return static_cast<int>(0);
    }

    return value;
}

long long MemoryUtility::ReadInt64(DWORD_PTR address) {
    if (address == 0) {
        return static_cast<long long>(0);
    }

    long long value = static_cast<long long>(0);
    __try {
        value = *(long long*)address;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return static_cast<long long>(0);
    }

    return value;
}

std::vector<unsigned char> MemoryUtility::ReadBytes(DWORD_PTR address, int length) {
    if (address == 0 || length <= 0) {
        return {};
    }

    std::vector<unsigned char> buffer(length, 0);

    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery((LPCVOID)address, &mbi, sizeof(mbi)) == 0) {
        return {}; // Memory region is invalid
    }

    // Check if the memory is readable
    if (!(mbi.Protect & PAGE_READONLY || mbi.Protect & PAGE_READWRITE || mbi.Protect & PAGE_EXECUTE_READ || mbi.Protect & PAGE_EXECUTE_READWRITE)) {
        return {};
    }

    // Read bytes safely
    for (int i = 0; i < length; ++i) {
        unsigned char* bytePtr = reinterpret_cast<unsigned char*>(address + i);
        if (IsBadReadPtr(bytePtr, sizeof(unsigned char))) {
            return {};
        }
        buffer[i] = *bytePtr;
    }

    return buffer;
}


double MemoryUtility::ReadDouble(DWORD_PTR address) {
    if (address == 0) return 0.0;
    double value = 0.0;
    __try {
        value = *(double*)address;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return 0.0;
    }
    return value;
}


DWORD_PTR MemoryUtility::GetPtrAddr(DWORD_PTR base, const std::vector<DWORD>& offsets) {
    DWORD_PTR addr = ReadInt32(base);  // Read the integer at the base address first
    for (size_t i = 0; i < offsets.size() - 1; ++i) {
        addr = ReadInt32(addr + offsets[i]);  // Read the integer at the intermediate addresses
    }
    return addr + offsets.back();  // Add the last offset directly to the final address
}


double MemoryUtility::ReadVideoTime() {
    if (tmodBaseAddress == 0) {
        return 0.0;
    }

    std::vector<DWORD> offsets = { 0x218 };
    DWORD_PTR base = tmodBaseAddress + 0x06C82304;
    DWORD_PTR timeAddr = GetPtrAddr(base, offsets);

    return ReadDouble(timeAddr);
}


double MemoryUtility::ReadMaxVideoTime() {
    if (tmodBaseAddress == 0) {
        return 0.0;
    }
    std::vector<DWORD> offsets = { 0x108, 0xA4, 0xE0 };
    DWORD_PTR base = tmodBaseAddress + 0x06C82304;
    DWORD_PTR timeAddr = GetPtrAddr(base, offsets);
    return ReadDouble(timeAddr);
}


static std::string url_decode(const std::string& encoded)
{
    int output_length;
    const auto decoded_value = curl_easy_unescape(nullptr, encoded.c_str(), static_cast<int>(encoded.length()), &output_length);
    std::string result(decoded_value, output_length);
    curl_free(decoded_value);
    return result;
}

std::vector<std::string> MemoryUtility::ReadVideoId() {
    if (modBaseAddress == 0) {
		return {};
	}
    std::vector<DWORD> offsets = { 0x24, 0x24, 0x8 };
    DWORD_PTR modBase = modBaseAddress + 0x003ED354;
    DWORD_PTR vidaddr = GetPtrAddr(modBase, offsets);

    int i = 1;
    std::vector<unsigned char> bytes;
    do {
        bytes = ReadBytes(vidaddr, i);
        ++i;
    } while (std::count(bytes.begin(), bytes.end(), '\x00') < 1);

    std::string str = ReadString(vidaddr, i);
    std::string rawVideoinfo = str.substr(str.find_last_of('/') + 1);
    std::string videoinfo = url_decode(rawVideoinfo);

    std::vector<std::string> result;
    std::stringstream ss(videoinfo);
    std::string token;
    while (std::getline(ss, token, ':')) {
        result.push_back(token);
    }

    return result;
}

// Function to read a string from the specified memory addressz
std::string MemoryUtility::ReadString(DWORD_PTR address, int length) {
    if (address == 0 || length <= 0) {
        return "";
    }

    if (!IsValidAddress(address)) {
        return "";
    }

    std::vector<char> buffer;
    for (int i = 0; i < length; ++i) {
        if (IsBadReadPtr((void*)(address + i), sizeof(char))) {
            break;
        }
        char byte = *(char*)(address + i);
        if (byte == '\x00') break; // Stop reading at the null byte
        buffer.push_back(byte);
    }

    return std::string(buffer.begin(), buffer.end());
}


DWORD_PTR MemoryUtility::GetModuleBaseAddress(DWORD processId, const std::wstring& moduleName) {
    MODULEENTRY32 moduleEntry;
    moduleEntry.dwSize = sizeof(MODULEENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    if (Module32First(snapshot, &moduleEntry)) {
        do {
            if (moduleName == moduleEntry.szModule) {
                CloseHandle(snapshot);
                return reinterpret_cast<DWORD_PTR>(moduleEntry.modBaseAddr);
            }
        } while (Module32Next(snapshot, &moduleEntry));
    }

    CloseHandle(snapshot);
    return 0;
}



bool MemoryUtility::IsValidAddress(DWORD_PTR address) {
    // Check if the address is not null
    if (address == 0) {
        return false;
    }

    // Try to read from the address
    __try {
        volatile char value = *(char*)address;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }

    return true;
}



void MemoryUtility::initialize()
{
    
    do {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        DWORD processId = GetCurrentProcessId();
        modBaseAddress = GetModuleBaseAddress(processId, L"EmbeddedBrowserWebView.dll");
        tmodBaseAddress = GetModuleBaseAddress(processId, L"libmpv-2.dll");

    } while (modBaseAddress == 0 || tmodBaseAddress == 0);
    

}