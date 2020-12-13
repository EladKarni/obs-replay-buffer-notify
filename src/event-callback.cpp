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
#include "event-callback.h"
#include "plugin-macros.generated.h"
}
#include "ipcplaybackcontroller.hpp"

void replay_buffer_saved_callback(enum obs_frontend_event event, void* ipc_controller_void) {
    //get the pointer to our controller
    IPCPlaybackController* ipc_controller = (IPCPlaybackController*)ipc_controller_void;
    
    //only activate if called with the proper event
    if(event == OBS_FRONTEND_EVENT_REPLAY_BUFFER_SAVED) {
        //log
        blog(LOG_INFO,"[replay_buffer_saved_callback]: registered replay buffer save event.");
        //play the sound using the controller under the given pointer
        auto[success,error_msg] = ipc_controller->play();
        if(!success) {
            blog(LOG_ERROR,"[replay_buffer_saved_callback]: IPC controller returned error upon trying to play the sound. Error message: %s",error_msg.toLocal8Bit().data());
        }
    }
}