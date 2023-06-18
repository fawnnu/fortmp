#pragma once

class scheduler_client
{
public:
    static void Initialize();

    static inline int Port;
};

struct gameserver_status
{
    gameserver_status()
    {
        accepting_players = false;
        number_of_players = 0;
        status = "offline";
        private_matchmaking_key = "";
        port = 0;
        max_players = 0;
        name = "";
        mode = "";
    }

    bool accepting_players;
    int number_of_players;
    int port;
    int max_players;
    std::string status;
    std::string private_matchmaking_key;
    std::string name;
    std::string mode;
};

class gameserver_client
{
public:
    gameserver_client();

    void Initialize();
    void Register();

    // Event called when a connection is established
    void OnConnected(websocketpp::connection_hdl handle);
    void OnMessageReceived(websocketpp::connection_hdl handle, websocketpp::config::asio_client::message_type::ptr message);

    // Send events
    void SendJoin();
    void SendUpdate();
    void SendLeave();
    void SendBan(std::string uuid);
    void SendSummary(std::string account_id, int Placement);
    void SendEventToChannel(std::string channel, std::string event_type, std::map<std::string, std::string> parameters);
    void SendEventToChannel(std::string channel, std::string event_type, nlohmann::json parameters);
    void SendHeartbeat();

    // Client WS object
    websocketpp::client<websocketpp::config::asio_tls_client> Client_WS;

    // Connection handle
    websocketpp::connection_hdl Connection_Handle;

    // GameServer status
    gameserver_status GameServer_Status;

    // UUID
    std::string UUID;
    
    // Message ref
    int MessageRef;

    // Static pointer to self
    inline static gameserver_client* Client;

    // Number of non-acknowledged local heartbeats
    int LocalNonAckHeartbeats;

    int LocalNonAckHeartbeatsForPlayers;
};