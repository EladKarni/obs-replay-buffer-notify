# OBS Replay Buffer Notify

A plugin for OBS Studio that gives an audible notification when the replay buffer is saved. It also give the ability to select the output device so the notification doesn't show up in a recording session

## Dependencies

### Plugin

* Qt Core (Inter-process communication)

### Playback host process

* Qt Core (Inter-process communication)
* cpptoml (header-only, for parsing the config file)
* SFML (reading audio samples from file)
* PortAudio (managing audio streams)
* tinycthread (probably header-only; Windows-only dependency due to Microsoft not implementing C11's `<thread.h>`)

## Building

Provide all of the build dependencies listed above (as well as `libobs` to be both  included and linked against) and then use CMake to compile the plugin.

## Installation

Take a look [here](HOW_TO_INSTALL_BUILDS.md).

## Todo

* Test playback with different hardware setups
* Consider CI integration
* Make sure that audio buffers are handled correctly with varying number of audio channels
