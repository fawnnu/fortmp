#pragma once

struct gameserver_status
{
    gameserver_status()
    {
        accepting_players = false;
        max_players = 0;
        number_of_players = 0;
        status = "offline";
    }

    bool accepting_players;
    int max_players;
    int number_of_players;
    std::string status;
    std::string private_matchmaking_key;
};

class gameserver_client
{
public:
    gameserver_client();

    // Event called when a connection is established
    void OnConnected(websocketpp::connection_hdl handle);
    void OnMessageReceived(websocketpp::connection_hdl handle, websocketpp::config::asio_client::message_type::ptr message);

    // Send events
    void SendJoin();
    void SendUpdate();
    void SendLeave();
    void SendHeartbeat();

    // Client WS object
    websocketpp::client<websocketpp::config::asio_tls_client> Client_WS;

    // Connection handle
    websocketpp::connection_hdl Connection_Handle;

    // GameServer status
    gameserver_status GameServer_Status;

    // Channel
    std::string Channel;
    
    // Message ref
    int MessageRef;

    // Static pointer to self
    inline static gameserver_client* Client;
};
