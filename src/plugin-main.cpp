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
#include <obs-module.h>
#include <obs-frontend-api.h>
#include <obs-hotkey.h>
#include "plugin-macros.generated.h"
#include "event-callback.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

/**returns if the module got loaded successfully
 *This declares our plugin for OBS
 */
bool obs_module_load(void);

/** This declares our plugin for OBS (pt. 2) */
void obs_module_unload();

} //extern "C"
#include "ipcplaybackcontroller.hpp"

/** used to manage the playback process */
IPCPlaybackController* ipc_controller;

bool obs_module_load(void)
{
    ipc_controller = nullptr;
    blog(LOG_INFO, "Creating child process...", PLUGIN_VERSION);
    ipc_controller = new IPCPlaybackController("obs_notification_service",QStringList());
    auto child_process_state = ipc_controller->getState();
    //make sure that the process did not crash
    if(child_process_state == QProcess::ProcessState::NotRunning) {
        //consider informing about the exit code
        blog(LOG_ERROR, "Failed to load: Could not start the child process.");
        delete ipc_controller;
        ipc_controller = nullptr;
        return false;
    }
    blog(LOG_INFO, "Child process launched. Issuing initialization...");
    auto[success,error_msg] = ipc_controller->init();
    if(!success) {
        blog(LOG_ERROR, "Failed to load: Child process reported error in initialization: %s",error_msg.toLocal8Bit().data());
        delete ipc_controller;
        ipc_controller = nullptr;
        return false;
    }
    blog(LOG_ERROR, "Child process has been initialized.");
    //register our event callback
    obs_frontend_add_event_callback(replay_buffer_saved_callback,(void*)ipc_controller);
    blog(LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);
    return true;
}


void obs_module_unload()
{   
    if(ipc_controller) {
        ipc_controller->exit();
        //The destructor of IPCPlaybackController is blocking and waits for 30secs for the playback process to exit
        delete ipc_controller;
    }
    //unregister our event callback
    obs_frontend_remove_event_callback(replay_buffer_saved_callback,(void*)ipc_controller);
    blog(LOG_INFO, "plugin unloaded");
}
