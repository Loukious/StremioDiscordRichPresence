#include "Discord.h"
#include <time.h>
#include <ctime>


static time_t g_startTime;

void Discord::initialize(bool setTimestamp)
{
    
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    
    // Initialize Discord RPC
    Discord_Initialize("997798118185771059", &handlers, 1, NULL);

    
    // Prepare rich presence struct
    DiscordRichPresence discordRichPresence;
    memset(&discordRichPresence, 0, sizeof(discordRichPresence));
    
    if (setTimestamp) {
        g_startTime = std::time(0);
    }
    discordRichPresence.startTimestamp = g_startTime;
    discordRichPresence.state = "In Menu";
    discordRichPresence.details = "Browsing catalog";
    discordRichPresence.largeImageKey = "https://avatars.githubusercontent.com/u/13152917";
    discordRichPresence.largeImageText = "Stremio";

    // Update Discord presence
    update(discordRichPresence);
}

void Discord::update(DiscordRichPresence discordRichPresence)
{
	Discord_UpdatePresence(&discordRichPresence);
}

time_t Discord::GetStartTime()
{
    return g_startTime;
}
