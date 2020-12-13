extern "C" {
#include "plugin-config.h"
#include "play-sound.h"
}
#include "jsoncommunication.hpp"
#include <iostream>
#include <memory>
#include <QByteArray>

// read the JSON message from stdin
inline JsonCommunication read_message() {
    std::string line_buf;
    std::getline(std::cin,line_buf);
    QByteArray arr(line_buf.c_str(),line_buf.size());
    JsonCommunication message(arr);
    return arr;
}

// write the JSON mesaage to stdout
inline void send_message(const JsonCommunication& message) {
    std::cout<<message.toJson().constData()<<'\n';
}

int main(int argc, char** argv) {
    int err = 0;
    //the received message
    std::unique_ptr<JsonCommunication> comm;
    try {
        comm.reset(new JsonCommunication(read_message()));
    }catch(...) {
        send_message(JsonCommunication::Error(QString("Failed to read the JSON message. Exiting.")));
        return 1;
    }
    if(comm->getCommand() == JsonCommunication::INIT) {
        err = Pa_Initialize();
        if(err) {
            send_message(JsonCommunication::Error(QString("Failed to initalize PortAudio. Error code: %1").arg(err)));
            Pa_Terminate();
            return 2;
        }
        PluginConfig* config = load_plugin_config(&err);
        if(err) {
            send_message(JsonCommunication::Error(QString("Failed to load plugin configuration. Error code: %1").arg(err)));
            free_plugin_config(config);
            Pa_Terminate();
            return 3;
        }
        send_message(JsonCommunication::Confirmation());
        do{
            try {
                comm.reset(new JsonCommunication(read_message()));
                if(comm->getCommand() == JsonCommunication::PLAY) {
                    play_sound(config->playback_context);
                    send_message(JsonCommunication::Confirmation());
                }
                if(comm->getCommand() == JsonCommunication::INIT) {
                    send_message(JsonCommunication::Error("Initialization request is inappriopriate when initialized already."));
                }
            }catch(...){
                send_message(JsonCommunication::Error(QString("Failed to read the JSON message. Ignoring.")));
            }
        }while(comm->getCommand() != JsonCommunication::QUIT);

        free_plugin_config(config);
        Pa_Terminate();
    } else { //unless the first received message is an initialization request
        send_message(JsonCommunication::Error("Unexpected message while unitialized"));
        return 4;
    }

}
