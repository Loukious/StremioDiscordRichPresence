#include <discord_register.h>
#include <discord_rpc.h>
#include <Windows.h>

class Discord {
public:
	static void initialize();
	static void update(DiscordRichPresence);
	static void clearPresence();
	static time_t GetStartTime();
};