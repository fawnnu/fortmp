#include "include.hpp"
#include "voice_component.hpp"

#include "hwid_component.hpp"
#include "phoenix_component.hpp"

voice_component::voice_component()
{
    auto NewPhoenixConnection = new phoenix_connection(XOR("VoiceChatConnection"));
    NewPhoenixConnection->Connect(std::string(XOR("wss://apollo.v1.erafn.org/voice/websocket?token=")) + hwid_component::access_token, OnOpen);
}

void voice_component::OnOpen(phoenix_connection* Connection)
{
}