#pragma once

typedef void(*OnClientConnectionOpen)(class eudp_server* server, class econnection* connection);
typedef void(*OnClientConnectionClose)(class eudp_server* server, class econnection* connection);
typedef void(*OnServerConnectionOpen)(class eudp_client* client);
typedef void(*OnServerEventCB)(class eudp_server* server, class econnection* connection, nlohmann::json payload);
typedef void(*OnClientEventCB)(class eudp_client* client, nlohmann::json payload);

enum class EConnectionNetworkStatus
{
    WaitingForHello = 0,
    WaitingForAck = 1,
    Open = 2
};

enum class EClientNetworkStatus
{
    WaitingForWelcome = 0,
    Open = 1
};

class econnection
{
public:
    econnection() = default;
    econnection(std::string in_key)
    {
        network_addr = sockaddr_in();
        network_status = EConnectionNetworkStatus::WaitingForHello;
        key = in_key;
        non_ack_heartbeats = 0;
        heartbeat_thread_id = std::string();
        owner = nullptr;
    }
    
    sockaddr_in network_addr;
    EConnectionNetworkStatus network_status;
    std::string key;
    int non_ack_heartbeats;
    std::string heartbeat_thread_id;
    eudp_server* owner;
};

class eudp_packet
{
public:
    eudp_packet() = default;
    eudp_packet(std::string InData)
    {
        Payload = InData;
        Size = InData.size();
    }
    eudp_packet(nlohmann::json InData)
    {
        Payload = InData.dump();
        Size = Payload.size();
    }
    
    std::string Payload;
    int Size;
};

class eudp_server
{
public:
    eudp_server() = default;
    eudp_server(int in_port, OnClientConnectionOpen InOnOpen);
    
    /*
     * Static function to handle all messages
     */
    static void HandleIncomingMessage(eudp_server* server, sockaddr_in client, char* Message);

    /*
     * Send message to client
     */
    bool Send(econnection* Client, eudp_packet Data);

    /*
     * Listen for custom event
     */
    void On(std::string EventName, econnection* connection, OnServerEventCB Callback);
 
    /*
     * Listen for one time custom event
     */
    void WaitFor(std::string EventName, econnection* connection, OnServerEventCB Callback);

    /*
     * Set close callback
     */
    void OnClose(OnClientConnectionClose CloseCB);
    
    /*
     * List of connections
     */
    std::vector<econnection*> connections;
    
    /*
     * Network socket
     */ 
    SOCKET server_socket;

    /*
     * Network port
     */
    int port;

    /*
     * Callback for when the connection to any client is established
     */
    OnClientConnectionOpen ClientConnectionOpenCB;
    
    /*
     * Callback for when the connection to any client is closed
     */
    OnClientConnectionClose ClientConnectionCloseCB;
 
    /*
     * Map of events and their callbacks
     */
    std::map<std::string, std::pair<OnServerEventCB, econnection*>> Events;
 
    /*
     * Map of one time events and their callbacks
     */
    std::map<std::string, std::pair<OnServerEventCB, econnection*>> OneTimeEvents;
};


class eudp_client
{
public:
    eudp_client() = default;
    eudp_client(std::string in_addr, int in_port, std::string in_key, OnServerConnectionOpen InOnServerConnectionOpen);

    /*
     * Static function to handle all messages
     */
    static void HandleIncomingMessage(eudp_client* client, char* Message);

    /*
     * Send message to server
     */
    bool Send(eudp_packet Data);
 
    /*
     * Listen for custom event
     */
    void On(std::string EventName, OnClientEventCB Callback);
 
    /*
     * Listen for custom event, once
     */
    void WaitFor(std::string EventName, OnClientEventCB Callback);
    
    /*
     * Close connection
     */
    void Close();

    /*
     * Server's IP address
     */
    std::string addr;

    /*
     * Server port
     */
    int port;

    /*
     * Server address formatted into a sockaddr_in instance
     */
    sockaddr_in server_addr;

    /*
     * Client socket
     */ 
    SOCKET client_socket;

    /*
     * Auth & Identifier key
     */
    std::string key;

    /*
     * Status of the connection
     */
    EClientNetworkStatus network_status;

    /*
     * Callback for when the connection is established
     */
    OnServerConnectionOpen ServerConnectionOpenCB;

    /*
     * Map of events and their callbacks
     */
    std::map<std::string, OnClientEventCB> Events;
 
    /*
     * Map of one time events and their callbacks
     */
    std::map<std::string, OnClientEventCB> OneTimeEvents;
};