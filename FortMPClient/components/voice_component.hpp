#pragma once

class voice_component
{
public:
    voice_component();

    static void OnOpen(class phoenix_connection* Connection);
    static void AudioThread();
    static int AudioCallback(const void* inputBuffer,
                 void* outputBuffer,
                 unsigned long framesPerBuffer,
                 const PaStreamCallbackTimeInfo* timeInfo,
                 PaStreamCallbackFlags statusFlags,
                 void* userData);

    static inline PaStream* InputOutputStream;
    static inline int bPendingShutdown;
};
