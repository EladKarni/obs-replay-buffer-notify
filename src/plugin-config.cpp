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
#include "plugin-config.h"
#include <stdlib.h>
}
#include <cpptoml.h>
#include <fstream>
#include <set>
#include <algorithm>

PluginConfig* load_plugin_config(int* error) {
    *error = 0;

    PluginConfig* ret = (PluginConfig*)malloc(sizeof(PluginConfig));
    ret->loaded_sound = nullptr;
    ret->sound_file_name = nullptr;
    ret->playback_context = nullptr;
    
    //get the target path pointing to the config file
    auto file_path = std::string(find_plugin_config_file());

    auto parse_or_create_config = [&file_path]() -> std::shared_ptr<cpptoml::table> {
        try{
            auto config = cpptoml::parse_file(file_path);
            if(!config->contains("audio_device_name") || !config->contains("sound_file"))
                throw std::runtime_error("Incomplete config");
            return config;
        }catch(...){ //when loading the config file fails for any reason: generate config in the target path
            std::ofstream new_config;
            new_config.open(file_path);
            new_config<<"#the sound file to be played"<<std::endl;
            new_config<<"sound_file = \"sound.wav\""<<std::endl;
            new_config<<"#the audio device to be used to play the sound. Use \"<default>\" to get the default one."<<std::endl;
            new_config<<"audio_device_name = \"<default>\""<<std::endl;
            new_config<<"#available output devices detected on this system by the PortAudio library:"<<std::endl;

            struct device_repr {
                std::string name;
                std::string host_api;
                bool default_device;
                //names that break the toml document need to be filtered out
                bool valid_name() {
                    return std::none_of(name.begin(),name.end(),[](char c){
                        return c == '\n' || c == '\r' || c == '\"';
                    });
                }
                //for nice sorting
                //without this operator, it is impossible to contain the struct in a std::set
                bool operator<(const device_repr& rhs) const {
                    if(host_api == rhs.host_api)
                        return name < rhs.name;
                    else 
                        return host_api < rhs.host_api;
                }
            };
            //this is used to get rid of possible duplicates
            std::set<device_repr> pa_devices;
            //load the PortAudio devices to the set
            int devices = Pa_GetDeviceCount();
            for(int i = 0; i < devices ; i++) { 
                auto device_info = Pa_GetDeviceInfo(i);
                device_repr device;
                device.default_device = (i == Pa_GetDefaultOutputDevice());
                device.name = std::string(device_info->name);
                device.host_api = Pa_GetHostApiInfo(device_info->hostApi)->name;
                if(device.valid_name())
                    pa_devices.insert(device);
            }
            //dump them to the config file
            for(const auto& x: pa_devices) {
                if(x.default_device)
                    new_config<<"#[default] ";
                else
                    new_config<<"#          ";
                new_config << '\"' << x.name << '\"' << "\t[host API: \"" << x.host_api << "\"]" << std::endl;
            }
            new_config<<"#end"<<std::endl; //just so that it's clear where it is considered finished
            new_config.close();
            auto config = cpptoml::parse_file(file_path);
            return config;
        }
    };

    //assigns a PaDeviceIndex to the name of the audio output device given in the config file
    auto resolve_device_from_name = [](const std::string& name) -> PaDeviceIndex {
        if(name == "<default>")
            return Pa_GetDefaultOutputDevice();
        int devices = Pa_GetDeviceCount();
        for(int i = 0; i < devices ; i++) {
                auto device_info = Pa_GetDeviceInfo(i);
                if(std::string(device_info->name) == name)
                    return i;
        }
        throw std::runtime_error("Failed to resolve output device name");
    };

    auto __std_string_to_c_string = [](const std::string& source) -> char* {
        char* ret;
        ret = (char*)malloc((source.size() + 1) * sizeof(char));
        strcpy(ret,source.c_str());
        return ret;
    };

    try{
        //this is a cpptoml library's TOML table
        auto config = parse_or_create_config();
        try{
        //initialize our C structure properly
            ret->sound_file_name = __std_string_to_c_string(*config->get_as<std::string>("sound_file"));
            ret->audio_device_num = resolve_device_from_name(*config->get_as<std::string>("audio_device_name"));
        } catch(...) {
            *error = DEVICE_NAME_RESOLUTION_ERROR; //couldn't resolve device from name
        }
    } catch(...) {
        *error = CONFIG_TOML_LOADING_ERROR; //couldn't parse /  create config 
    }

    //only load the sound file if no error was reported up to this point
    if(*error == 0)
        ret->loaded_sound = load_sound(ret->sound_file_name,error);
    //only initalize the playback context if everything is ok up to this point
    if(*error == 0)
        ret->playback_context = load_playback_context(ret->loaded_sound,ret->audio_device_num,error);

    return ret;
}

const char* find_plugin_config_file() {
    //Currently it just points to "config.toml" in the current working dir
    //this can be expanded and made smarter
    return "./config.toml";
}

void free_plugin_config(PluginConfig* config) {
    if(config->playback_context)
        free_playback_context(config->playback_context);
    if(config->loaded_sound)
        free_sound(config->loaded_sound);
    if(config->sound_file_name)
        free(config->sound_file_name);
    free(config);
}