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

struct reply_info
{
    reply_info(PVOID InCallback, std::string InChannel, nlohmann::json InPayload)
    {
        Callback = InCallback;
        Channel = InChannel;
        Payload = InPayload;
    }
    
    PVOID Callback;
    std::string Channel;
    nlohmann::json Payload;
};

class gameserver_client
{
public:
    gameserver_client();

    void Initialize();
    void Register();
    static void WaitForReply(int MessageReference, PVOID callback);

    // Event called when a connection is established
    void OnConnected(websocketpp::connection_hdl handle);
    void OnMessageReceived(websocketpp::connection_hdl handle, websocketpp::config::asio_client::message_type::ptr message);

    // Send events
    int SendJoin();
    int SendUpdate();
    int SendLeave();
    int SendBan(std::string uuid);
    int SendEventToChannel(std::string channel, std::string event_type, std::map<std::string, std::string> parameters);
    int SendEventToChannel(std::string channel, std::string event_type, nlohmann::json parameters);
    int SendHeartbeat();

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

    // Map for messages and their callbacks
    std::map<int, PVOID> MessagesMap;

    // Last message sent
    nlohmann::json LastMessagePayload;

    int LocalNonAckHeartbeatsForPlayers;
};