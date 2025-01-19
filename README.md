# Discord Rich Presence for Stremio
This is a simple mod that adds Discord Rich Presence support to Stremio. It shows the movie or series you are watching on Discord.

## Features
- Shows the movie or series you are watching on Discord.
- Shows the current time of the movie or series.
- Shows the episode number and season of the series you are watching (in case of a series).
- Shows when you are paused (Optional).
- Shows when you are in the main menu (Optional).
- Shows thumbnail of the movie or series you are watching.
- Shows a smaller thumbnail of the episode you are watching (in case of a series).
- Shows a button to see the movie or series on IMDb (Optional).
- Shows a button to see the movie or series on Stremio Web (Optional).
- Configurable settings.

## Screenshots
![General view playing](https://i.imgur.com/q0u4BFn.png)
![General view paused](https://i.imgur.com/AKG0Bi0.png)
![Small icon shows episode name](https://i.imgur.com/WZ0A887.png)
![Big icon shows movie/series name](https://i.imgur.com/CXIPc7R.png)


## Steps to install
1. Locate the Stremio installation folder.
> By default, for Stremio 4 it is `%localappdata%\Programs\LNV\Stremio-4`.

> For Stremio 5 it is `%localappdata%\Programs\StremioService`.
2. Download the zip file for [Stremio 5](../../releases/latest) or [Stremio 4](../../releases/tag/v0.0.4) and extract it in your Stremio folder.
3. Verify that your Discord Rich Presence is working.

## Configuring the mod
1. Open the `RPCconfig.ini` file in the Stremio folder.
2. Change the settings as you like.
3. Save the file.

## Steps to uninstall
1. Delete the `DWrite.dll` file from the Stremio folder.

> ⚠️ **Warning:** Please note that after updating Stremio, the DWrite.dll file might break and will cause Stremio to crash. Make sure to check for updates or reapply the necessary changes to ensure compatibility.
# Support
[!["Buy Me A Coffee"](https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png)](https://buymeacoffee.com/loukious)

## Build rerequisites
- Visual Studio 2019 or later
