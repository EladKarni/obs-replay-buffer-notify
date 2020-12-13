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

extern "C" {
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <portaudio.h>
#include "play-sound.h"
}
#include <SFML/Audio.hpp>

//a simple struct to carry the playback data for the portaudio callback
typedef struct {
    int64_t current_sample;
    int64_t total_samples;
    //array of samples with size channel_count * total_samples
    signed short* samples;
    int channel_count;
} PaPlaybackData;

//a portaudio stream callback. See some PortAudio example codes for reference.
static int stream_callback( const void *inputBuffer, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData )
{
    PaPlaybackData *data = (PaPlaybackData*)userData;
    signed short *out = (signed short*)outputBuffer;
    unsigned long i;

    (void) timeInfo; /* Prevent unused variable warnings. */
    (void) statusFlags;
    (void) inputBuffer;

    for(i = 0; i<framesPerBuffer; i++ )
    {
        for(int j = 0; j < data->channel_count; j++ )
            *out++ = data->samples[data->current_sample++];  
        //end the playback when appriopriate
        if(data->current_sample >= data->total_samples * data->channel_count) 
            return paComplete;
    }

    return paContinue;
}

//forward declared: definition below
int playback_context_thread_func(void *arg);

PlaybackContext* load_playback_context(LoadedSound* loaded_sound, PaDeviceIndex pa_device, int* error) {
    *error = 0;
    PlaybackContext* ret = (PlaybackContext*)malloc(sizeof(PlaybackContext));
    ret->sound = loaded_sound;
    //PortAudio internal struct to set all required parameters of the stream
    ret->output_params = (PaStreamParameters*)malloc(sizeof(PaStreamParameters));
    ret->internal_callback_data = malloc(sizeof(PaPlaybackData));
    ret->playback_thread_mutex = nullptr;
    ret->playback_thread = nullptr;
    ret->playback_thread_pause = nullptr;
    //for convenience
    PaPlaybackData* data = (PaPlaybackData*)ret->internal_callback_data;
    //initialize current playing position
    data->current_sample = 0;

    //reference the loaded sound's memory directly in the temporary data structure
    data->total_samples = ret->sound->total_samples;
    data->samples = ret->sound->samples;
    data->channel_count = ret->sound->channel_count;

    //set the PortAudio parameters
    ret->output_params->device = pa_device;
    ret->output_params->channelCount = ret->sound->channel_count;       
    ret->output_params->sampleFormat = paInt16; 
    ret->output_params->suggestedLatency = Pa_GetDeviceInfo(ret->output_params->device)->defaultLowOutputLatency;
    ret->output_params->hostApiSpecificStreamInfo = NULL;

     //initalizes output_stream
    *error = Pa_OpenStream(
        &ret->output_stream,
        NULL, //no input stream
        ret->output_params, //parameters for the stream
        (double) ret->sound->sample_rate,
        512, //frames per buffer
        paClipOff, //PortAudio stream flags
        stream_callback, //the callback defined above, used for the playback
        ret->internal_callback_data //reference the struct that carries playback info
    );
 
    //initialize threading state before launching a new thread
    ret->playback_thread_quit = false;
    ret->playback_active = false;

    ret->playback_thread = (thrd_t*)malloc(sizeof(thrd_t));
    ret->playback_thread_mutex = (mtx_t*)malloc(sizeof(mtx_t));
    ret->playback_thread_pause = (cnd_t*)malloc(sizeof(cnd_t));

    cnd_init(ret->playback_thread_pause);
    mtx_init(ret->playback_thread_mutex,mtx_plain);
    thrd_create(ret->playback_thread,playback_context_thread_func,ret);

    return ret;
}

//starts and stops playback
void play_sound_impl(PlaybackContext* ctx) {
    PaPlaybackData* data = (PaPlaybackData*)ctx->internal_callback_data;
    data->current_sample = 0;
    //start the stream
    Pa_StartStream(ctx->output_stream);
    //wait for it to play
    Pa_Sleep((double)ctx->sound->total_samples / (double)ctx->sound->sample_rate / (double)ctx->sound->channel_count * 1000);
    //stio the strem
    Pa_StopStream(ctx->output_stream);
}

void play_sound(PlaybackContext* ctx) {
    
    mtx_lock(ctx->playback_thread_mutex);
    bool active = ctx->playback_active;
    mtx_unlock(ctx->playback_thread_mutex);
    //we do not want to do anything if the sound is currently being played
    if(!active) {
        mtx_lock(ctx->playback_thread_mutex);
        //this confirms that the condition variable wake-up (below) is not spurious
        ctx->playback_active = true;
        mtx_unlock(ctx->playback_thread_mutex);
        //notify the playback thread
        cnd_signal(ctx->playback_thread_pause);
    }
}

//the function to be run in the playback thread
int playback_context_thread_func(void *arg) {
    PlaybackContext* ctx = (PlaybackContext*)arg;
    mtx_lock(ctx->playback_thread_mutex);
    //loop until it's time to deinitialize
    while(!ctx->playback_thread_quit) {
        //pauses the thread until any action is relevant
        while(!ctx->playback_active && !ctx->playback_thread_quit) {
            cnd_wait(ctx->playback_thread_pause,ctx->playback_thread_mutex);
        }
        //stop looping when it's time to quit
        if(ctx->playback_thread_quit)
            break;
        //the mutex has to be unlocked not to block access for the time of playback
        mtx_unlock(ctx->playback_thread_mutex);
        //start playing the sound and wait for it to complete
        play_sound_impl(ctx);

        mtx_lock(ctx->playback_thread_mutex);
        //notify that the playback has finished
        ctx->playback_active = false;
    }
    mtx_unlock(ctx->playback_thread_mutex);
    thrd_exit(0);
    //required for MSVC
    return 0;
}

LoadedSound* load_sound(const char* path, int* error) {
    LoadedSound* ret = (LoadedSound*)malloc(sizeof(LoadedSound));
    ret->samples = nullptr;

    //use SFML to read the audio file given by path
    sf::SoundBuffer audio_buf;
    audio_buf.loadFromFile(path);

    auto* audio_buf_samples = audio_buf.getSamples();
    ret->samples = (signed short*)malloc(audio_buf.getSampleCount() * audio_buf.getChannelCount() * sizeof(signed short));
    //copy the samples directly to the plugin's internal audio data representation
    memcpy(ret->samples,audio_buf_samples,audio_buf.getSampleCount() * audio_buf.getChannelCount());
    //copy the rest of audio data
    ret->channel_count = audio_buf.getChannelCount();
    ret->sample_rate = audio_buf.getSampleRate();
    ret->total_samples = audio_buf.getSampleCount();
    
    *error = 0;

    return ret;
}

void free_sound(LoadedSound* sound) {
    if(sound->samples)
        free(sound->samples);
    free(sound);
}



void free_playback_context(PlaybackContext* ctx) {
    mtx_lock(ctx->playback_thread_mutex);
    //for the playback thread to know it has to join
    ctx->playback_thread_quit = true;
    ctx->playback_active = false;
    mtx_unlock(ctx->playback_thread_mutex);

    //unpause the playback thread
    cnd_broadcast(ctx->playback_thread_pause);
    thrd_join(*ctx->playback_thread,NULL);

    //free threading memory
    free(ctx->playback_thread);
    cnd_destroy(ctx->playback_thread_pause);
    mtx_destroy(ctx->playback_thread_mutex);
    free(ctx->playback_thread_pause);
    free(ctx->playback_thread_mutex);

     //deallocates output_stream
    Pa_CloseStream(ctx->output_stream);
    PaPlaybackData* m_data = (PaPlaybackData*)ctx->internal_callback_data;
    free(m_data);
    free(ctx->output_params);
    //do not deallocate the sound please!
    free(ctx);
}