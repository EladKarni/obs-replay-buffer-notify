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

#ifndef PLAY_SOUND_H
#define PLAY_SOUND_H
#include <portaudio.h>
#ifdef WIN32
//Microsoft doesn't care about C standard
#include <tinycthread.h>
#else
#include <threads.h>
#endif

/** Holds the loaded sound in memory
 * 
 */
struct LoadedSound {
    unsigned int sample_rate;
    long total_samples;
    int channel_count;
    /** array of samples with size total_samples * channel_count */
    signed short* samples;
};
typedef struct LoadedSound LoadedSound;

/** facility to manage the playback thread as well as PortAudio's sound stream status
 */
struct PlaybackContext
{
    /** A pointer referencing the target sound */
    LoadedSound* sound;

    //PortAudio status

    PaStreamParameters* output_params;
    PaStream* output_stream;
    
    /** implementation-defined data to be passed to a PortAudio callback */
    void* internal_callback_data;

    //Threading data

    /** true if currently playing audio */
    bool playback_active;
    /** true if it's time for the playback thread to join */
    bool playback_thread_quit;

    /** the thread dedicated for playback */
    thrd_t* playback_thread;
    /** used to pause the playback thread */
    cnd_t* playback_thread_pause;
    /** ensures thread-safe access */
    mtx_t* playback_thread_mutex;
};
typedef struct PlaybackContext PlaybackContext;

/** triggers playback in a separate thread (unless the sound is being played already).
 * Does not block.
 */
void play_sound(PlaybackContext*);

/** uses SFML to read a sound file to memory from the specified path
 *  supported formats include: .ogg (vorbis), .flac, .wav
 * 
 *  SFML is bad at error handling so this probably cannot report an error in reality
 */
LoadedSound* load_sound(const char* path, int* error);

/** frees the loaded sound */
void free_sound(LoadedSound*);

/** Creates a playback context with a given sound. 
* Does not take ownership of the sound.
* The sound should not be accessed or freed for the whole playback context's lifetime.
* In case of initialization failure, the returned pointer needs to be freed.
* Requires the PortAudio library to be initialized.
*/
PlaybackContext* load_playback_context(LoadedSound* sound,PaDeviceIndex target_device,int* error);

/** destroys and frees the given context.
 *  Will block if the sound is currently being played/
 */
void free_playback_context(PlaybackContext*);

#endif