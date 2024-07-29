#pragma once

#pragma comment(linker,"/export:DWriteCreateFactory=C:\\Windows\\SysWOW64\\DWrite.DWriteCreateFactory,@1")

#include "windows.h"
#include "ios"
#include "fstream"
#include "Discord.h"

#include "MemoryUtility.h"
#include <mutex>
#include <psapi.h>
#include <format>
#include "Video.h"


static Discord* g_discord;
static MemoryUtility* g_memoryUtility;
static bool threadStarted = false;
static HANDLE hMainThread = NULL;
static std::mutex initMutex;

// Remove this line if you aren't proxying any functions.
HMODULE hModule = LoadLibrary(L"C:\\Windows\\SysWOW64\\DWrite.dll");


//void Logs(const std::string& message)
//{
//	std::ofstream log("spartacus-proxy-DWrite.log", std::ios_base::app | std::ios_base::out);
//	log << message;
//	log << "\n";
//}


static void UpdateDiscordPresence() {
	std::string oldinfo = "";

	while (true) {
		double videoTime = g_memoryUtility->ReadVideoTime();

		if (videoTime > 0) {
			Video vid;
			DiscordRichPresence discordRichPresence;

			std::vector<std::string> videoId = vid.getvidinfo();
			std::string currentInfoStr = vid.join(videoId, ":");
			if (oldinfo != currentInfoStr) {
				oldinfo = currentInfoStr;
				vid = Video(); // Reinitialize video object with new video info
			}

			std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			int difference = static_cast<int>(currentTime) - static_cast<int>(videoTime);

			std::string state;
			std::string details;
			std::string largeImageKey;
			std::string largeImageText;
			std::string smallImageKey;
			std::string smallImageText;
			std::string IMBdUrl = "https://www.imdb.com/title/" + vid.id;
			std::string StremioUrl = "https://web.stremio.com/#/detail/" + vid.type + "/" + vid.id + "/" + currentInfoStr;

			if (vid.type == "series") {
				state = vid.name + " (S" + vid.s + "-E" + vid.ep + ")";
				details = "Watching Series";
				largeImageKey = vid.poster;
				largeImageText = vid.name + " " + vid.year;
				smallImageKey = vid.thumbnail;
				smallImageText = vid.epname;
				discordRichPresence.startTimestamp = difference;
			}
			else {
				state = vid.name + " " + vid.year;
				details = "Watching a Movie";
				largeImageKey = vid.poster;
				largeImageText = vid.name + " " + vid.year;
				discordRichPresence.startTimestamp = difference;
			}

			discordRichPresence.state = state.c_str();
			discordRichPresence.details = details.c_str();
			discordRichPresence.largeImageKey = largeImageKey.c_str();
			discordRichPresence.largeImageText = largeImageText.c_str();
			discordRichPresence.smallImageKey = smallImageKey.c_str();
			discordRichPresence.smallImageText = smallImageText.c_str();
			discordRichPresence.button1Label = "More details";
			discordRichPresence.button1Url = IMBdUrl.c_str();
			discordRichPresence.button2Label = "Watch on Stremio";
			discordRichPresence.button2Url = StremioUrl.c_str();


			g_discord->update(discordRichPresence);
		}
		else {
			g_discord->initialize(false);
		}

		std::this_thread::sleep_for(std::chrono::seconds(10));
	}
}





DWORD WINAPI MainThread(LPVOID param) {

    g_discord->initialize();
	
	g_memoryUtility->initialize();


	// Create a thread to update Discord presence
	HANDLE hThread = CreateThread(
		NULL,            // default security attributes
		0,               // use default stack size
		[](LPVOID) -> DWORD {
			UpdateDiscordPresence();
			return 0;
		},
		NULL,            // argument to thread function
		0,               // use default creation flags
		NULL);           // returns the thread identifier

	if (hThread == NULL) {
		DWORD dwError = GetLastError();
	}
	else {
		CloseHandle(hThread);
	}

	return 0;

}

std::string GetProcessName()
{
	wchar_t processName[MAX_PATH] = { 0 };
	DWORD processId = GetCurrentProcessId();
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);

	if (hProcess)
	{
		HMODULE hMod;
		DWORD cbNeeded;
		if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
		{
			GetModuleBaseNameW(hProcess, hMod, processName, sizeof(processName) / sizeof(wchar_t));
		}
		CloseHandle(hProcess); // Move CloseHandle inside the if block
	}

	// Convert wide string to narrow string using wcstombs_s
	char narrowProcessName[MAX_PATH];
	size_t convertedChars = 0;
	wcstombs_s(&convertedChars, narrowProcessName, MAX_PATH, processName, _TRUNCATE);

	// Ensure null termination
	narrowProcessName[MAX_PATH - 1] = '\0';

	return std::string(narrowProcessName);
}




BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	std::lock_guard<std::mutex> lock(initMutex);

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		if (GetProcessName() == "stremio.exe") {
			if (!threadStarted) {
				hMainThread = CreateThread(NULL, 0, MainThread, NULL, 0, NULL);
				if (hMainThread != NULL) {
					threadStarted = true;
				}
			}
		}
		break;
	case DLL_PROCESS_DETACH:
		if (hMainThread != NULL) {
			CloseHandle(hMainThread);
			hMainThread = NULL;
			threadStarted = false;
		}
		break;
	}
	return TRUE;
}
