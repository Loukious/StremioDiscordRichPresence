#include <discord_register.h>
#include <discord_rpc.h>
#include <Windows.h>

class Discord {
public:
	static void initialize(bool setTimestamp = true);
	static void update(DiscordRichPresence);
	static time_t GetStartTime();
};