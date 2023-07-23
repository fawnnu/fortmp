#include "include.hpp"
#include "voice_component.hpp"

#include "hwid_component.hpp"
#include "phoenix_component.hpp"

voice_component::voice_component()
{
    auto NewPhoenixConnection = new phoenix_connection(XOR("VoiceChatConnection"));
    NewPhoenixConnection->Connect(std::string(XOR("wss://apollo.v1.erafn.org/voice/websocket?token=")) + hwid_component::access_token + XOR("&vsn=2.0.0"), OnOpen);

    // Start capturing audio
    CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(AudioThread), nullptr, 0, nullptr);
}

int voice_component::AudioCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
{
    // Handle I/O
    
    return paContinue;   
}

void voice_component::AudioThread()
{
    if (Pa_Initialize() != paNoError)
    {
        // Error
        return;
    }

    // Create an I/O stream to handle both capturing and reproduction of audio
    if (Pa_OpenDefaultStream(&InputOutputStream, 1, 1, paFloat32, 48000, 16, AudioCallback, nullptr) != paNoError)
    {
        // Error
        return;
    }

    if (Pa_StartStream(InputOutputStream) != paNoError)
    {
        // Error
        return;
    }

    // Capture audio until bPendingShutdown is true;
    while (bPendingShutdown == false)
    {
        Sleep(1000 / 30);
    }
    
    // Close streams and terminate PortAudio
    Pa_StopStream(InputOutputStream);
    Pa_CloseStream(InputOutputStream);
    Pa_Terminate();
}

void voice_component::OnOpen(phoenix_connection* Connection)
{
    phoenix_channel* VoiceChannel = Connection->Channel(XOR("voice:") + hwid_component::account_id);
    VoiceChannel->Push(XOR("phx_join"),{}).Receive([](phoenix_channel* channel, nlohmann::json response)
    {
        channel->On(XOR("connect"), [](phoenix_channel* channel, nlohmann::json response)
        {
            if (response.contains(XOR("party")))
            {
                std::string PartyID = response[XOR("party")];
                phoenix_channel* PartyChannel = channel->OwnerConnection->Channel(XOR("party:") + PartyID);
                PartyChannel->Push(XOR("phx_join"), {
                }).Receive([](phoenix_channel* channel, nlohmann::json response)
                {
                    
                });
            }
        });
    });
}