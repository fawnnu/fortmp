#pragma once

class voice_component
{
public:
    voice_component();

    static void OnOpen(class phoenix_connection* Connection);
    static void AudioThread();
};
