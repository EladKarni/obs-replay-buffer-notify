/*
OBS-ReplayBuffer-Notification
Copyright (C) 2020 Jakub Smulski <hgonomeg@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#ifndef PLUGIN_CONFIG_H
#define PLUGIN_CONFIG_H
#include "play-sound.h"

/** Holds the runtime configuration options as well as the sound file for the plugin
 * 
 *  
 */
struct PluginConfig 
{
    /** sound file to be played/
     * supported formats include: .ogg (vorbis), .flac, .wav
    */
    char* sound_file_name;
    /** runtime storage: loaded sound file */
    LoadedSound* loaded_sound;
    /** a structure managing the playback thread and audio streams */
    PlaybackContext* playback_context;
    /** the index of the audio device, as given by the PortAudio library */
    PaDeviceIndex audio_device_num;
};

typedef struct PluginConfig PluginConfig;

/** Attempt loading the plugin configuration as well as the necessary resources (the sound file).
  * Needs to be freed in case of failure.
  * Requires the PortAudio library to be initialized
  * 
  * There are many possible error codes.
  * Negative ones probably come from the PortAudio library.
  */
PluginConfig* load_plugin_config(int* error);
#define DEVICE_NAME_RESOLUTION_ERROR 17; /**< the device name given in the config could not be resolved or the device could not be found either way */
#define CONFIG_TOML_LOADING_ERROR 11; /**< couldn't parse or create a valid config */


/** Frees the allocated config.
* Will block if the audio is currently being played.
*/
void free_plugin_config(PluginConfig* config);

/** Find the TOML config file. 
 * Currently it just points to "config.toml" in the current working dir.
*/
const char* find_plugin_config_file();

#endif