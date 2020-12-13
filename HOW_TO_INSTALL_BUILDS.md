# General

The plugin consists of two main components:

* The dynamically loaded library file (.dll (or .so on Linux)) which is the OBS plugin file
* The `obs_notification_service` executable file which hosts audio playback

The plugin file should be placed in the directory where OBS plugins are kept.
For instance on 64bit Windows builds at `C:\Program Files\OBS\plugins` assuming OBS was installed at `C:\Program Files\OBS`. On Linux systems this is likely at `/usr/lib/obs-plugins/`.

The audio playback service executable should be put in the OBS executable's working directory.

# Library dependencies

The plugin does not have any additional dependencies. The audio playback service however depends upon the following libraries not shipped with OBS:

* SFML (audio and system modules)
* PortAudio

On Linux systems you will have them installed in your system if you had built the plugin from the source code.
For Windows, those dependencies need to be placed in the OBS executable's directory.

# Bundled build

This message comes with a bundled 64-bit Windows build inside the `build_x64.7z` archive.
It contains the plugin, the playback service and the dependencies as well.

`obs-replaybuffer-notification.dll` is the OBS plugin and should go to the plugins folder as described above.
`obs_notification_service.exe` is the executable that hosts playback. All the remaining .dll files are its' dependencies.


# Troubleshooting

This plugin requires OBS version >26 to operate.

Have a thorough look at OBS' logs to diagnose if the plugin is found.
If so, look for potential error messages to help diagnose the problem.
