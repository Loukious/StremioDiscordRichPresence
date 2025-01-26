#pragma once

#pragma comment(linker,"/export:DWriteCreateFactory=C:\\Windows\\SysWOW64\\DWrite.DWriteCreateFactory,@1")

#include "windows.h"
#include "ios"
#include "fstream"
#include "Discord.h"
#include "ini.h"
#include "MemoryUtility.h"
#include <mutex>
#include <psapi.h>
#include <format>
#include "Video.h"
#include <iostream>
using namespace mINI;


static Discord* g_discord;
static MemoryUtility* g_memoryUtility;
static bool threadStarted = false;
static HANDLE hMainThread = NULL;
static std::mutex initMutex;

// Remove this line if you aren't proxying any functions.
HMODULE hModule = LoadLibrary(L"C:\\Windows\\SysWOW64\\DWrite.dll");

static bool show_buttons = true;
static std::string link_target = "app";
static bool disable_in_menu = false;
static bool disable_when_paused = false;
static bool enable_console = false;
static int refresh_interval = 10;
static time_t g_startTime;


static std::string toLowerCase(const std::string& str) {
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(), ::tolower);
	return result;
}

static void ReadConfig()
{
	INIFile config(".\\RPCconfig.ini");
	INIStructure ini;


	if (config.read(ini)) {
		// Successfully read the INI file, load the values
		show_buttons = ini["Buttons"]["show_buttons"] == "true";
		link_target = toLowerCase(ini["Buttons"]["link_target"]);
		if (link_target != "app" && link_target != "web") {
			link_target = "app"; // Default to "app" if invalid
		}
		disable_in_menu = ini["Activity"]["disable_in_menu"] == "true";
		disable_when_paused = ini["Activity"]["disable_when_paused"] == "true";
		enable_console = ini["Debug"]["enable_console"] == "true";
		refresh_interval = std::stoi(ini["Activity"]["refresh_interval"]);
		if (refresh_interval < 1 || refresh_interval > 60) {
			refresh_interval = 10; // Default to 10 seconds if invalid
		}

	}
	else {
		// Failed to read the INI file, initialize with default values and create the file
		ini["Buttons"]["show_buttons"] = show_buttons ? "true" : "false";
		ini["Buttons"]["link_target"] = link_target;
		ini["Activity"]["disable_in_menu"] = disable_in_menu ? "true" : "false";
		ini["Activity"]["disable_when_paused"] = disable_when_paused ? "true" : "false";
		ini["Debug"]["enable_console"] = enable_console ? "true" : "false";
		ini["Activity"]["refresh_interval"] = std::to_string(refresh_interval);

		// Write default configuration to file
		if (!config.write(ini, true)) {
			std::cerr << "Error: Unable to write default configuration to RPCconfig.ini" << std::endl;
		}
	}

}

static void UpdateDiscordPresence() {
	std::string oldinfo = "";
	double previousVideoTime = -1;
	bool isPaused = false;

	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(refresh_interval));

		double videoTime = g_memoryUtility->ReadVideoTime();

		if (videoTime > 0) {
			Video vid;
			DiscordRichPresence discordRichPresence;
			memset(&discordRichPresence, 0, sizeof(discordRichPresence));
			std::vector<std::string> videoId = vid.getvidinfo();
			std::string currentInfoStr = vid.join(videoId, ":");
			if (oldinfo != currentInfoStr) {
				oldinfo = currentInfoStr;
				vid = Video(); // Reinitialize video object with new video info
			}

			if (videoTime == previousVideoTime) {
				isPaused = true;
			}
			else {
				isPaused = false;
				previousVideoTime = videoTime;
			}

			std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			int difference = static_cast<int>(currentTime) - static_cast<int>(videoTime);
			std::time_t endTime = currentTime + static_cast<int>(vid.getmaxtime() - vid.gettime());

			std::string state;
			std::string details = vid.name;
			std::string largeImageKey = vid.poster;
			std::string largeImageText = vid.name + " " + vid.year;
			std::string smallImageKey;
			std::string smallImageText;
			std::string IMBdUrl = "https://www.imdb.com/title/" + vid.id;
			std::string StremioUrl = "";
			if (link_target == "web") {
				StremioUrl = "https://web.stremio.com/#/detail/" + vid.type + "/" + vid.id + "/" + currentInfoStr;
			}
			else {
				StremioUrl = "stremio:///detail/" + vid.type + "/" + vid.id + "/" + currentInfoStr;
			}

			if (isPaused) {
				if (disable_when_paused) {
					g_discord->clearPresence();
					continue;
				}
				smallImageText = "Paused";
				smallImageKey = "https://i.imgur.com/eCUJpm9.png";
			}
			else {
				smallImageText = "Playing";
				smallImageKey = "https://raw.githubusercontent.com/Stremio/stremio-web/refs/heads/development/images/icon.png";
				discordRichPresence.startTimestamp = difference;
				discordRichPresence.endTimestamp = endTime;
			}

			discordRichPresence.type = DISCORD_ACTIVITY_TYPE_WATCHING;
			discordRichPresence.state = state.c_str();
			discordRichPresence.details = details.c_str();
			discordRichPresence.largeImageKey = largeImageKey.c_str();
			discordRichPresence.largeImageText = largeImageText.c_str();
			discordRichPresence.smallImageKey = smallImageKey.c_str();
			discordRichPresence.smallImageText = smallImageText.c_str();
			if (show_buttons) {
				discordRichPresence.button1Label = "More details";
				discordRichPresence.button1Url = IMBdUrl.c_str();
				discordRichPresence.button2Label = "Watch on Stremio";
				discordRichPresence.button2Url = StremioUrl.c_str();
			}
			g_discord->update(discordRichPresence);
		}
		else if (!disable_in_menu) {
			DiscordRichPresence discordRichPresence;
			memset(&discordRichPresence, 0, sizeof(discordRichPresence));
			discordRichPresence.type = DISCORD_ACTIVITY_TYPE_WATCHING;
			discordRichPresence.state = "In Menu";
			discordRichPresence.details = "Browsing catalog";
			discordRichPresence.largeImageKey = "https://raw.githubusercontent.com/Stremio/stremio-web/refs/heads/development/images/icon.png";
			discordRichPresence.largeImageText = "Stremio";
			discordRichPresence.smallImageKey = "https://raw.githubusercontent.com/Stremio/stremio-web/refs/heads/development/images/icon.png";
			discordRichPresence.smallImageText = "Stremio";
			discordRichPresence.startTimestamp = g_startTime;
			g_discord->update(discordRichPresence);
		}
		else {
			g_discord->clearPresence();
		}
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
			ReadConfig();
			g_startTime = std::time(0);
			if (enable_console) {
				AllocConsole(); // Allocate a console
				freopen_s((FILE**)stdout, "CONOUT$", "w", stdout); // Redirect stdout to console
				freopen_s((FILE**)stderr, "CONOUT$", "w", stderr); // Redirect stderr to console
				freopen_s((FILE**)stdin, "CONIN$", "r", stdin);    // Redirect stdin to console

				std::cout << "DLL attached to process: " << GetProcessName() << std::endl;
			}

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
		FreeConsole();
		break;
	}
	return TRUE;
}
