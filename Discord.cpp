#include "Discord.h"
#include <time.h>
#include <ctime>


static time_t g_startTime;

void Discord::initialize()
{
    
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    
    // Initialize Discord RPC
    Discord_Initialize("997798118185771059", &handlers, 1, NULL);

}

void Discord::update(DiscordRichPresence discordRichPresence)
{
	Discord_UpdatePresence(&discordRichPresence);
}

void Discord::clearPresence()
{
	Discord_ClearPresence();
}


time_t Discord::GetStartTime()
{
    return g_startTime;
}
