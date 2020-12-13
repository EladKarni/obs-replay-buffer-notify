# ReplayBuffer-Notification

Replay Buffer notification sound plugin for OBS

## Dependencies

### Plugin

* Qt Core (Inter-process communication)

### Playback host process

* Qt Core (Inter-process communication)
* cpptoml (header-only, for parsing the config file)
* SFML (reading audio samples from file)
* PortAudio (managing audio streams)
* tinycthread (probably header-only; Windows-only dependency due to Microsoft not implementing C11's `<thread.h>`)

## Todo

* Test playback with different hardware setups
* Consider CI integration
* Make sure that audio buffers are handled correctly with varying number of audio channels