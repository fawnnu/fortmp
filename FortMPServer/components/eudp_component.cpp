#include "include.hpp"
#include "eudp_component.hpp"

#ifndef LOG
#define LOG(msg, ...) spdlog::info(msg, __VA_ARGS__); // Define log to your own liking
#endif

using namespace nlohmann;

eudp_server::eudp_server(int in_port, OnClientConnectionOpen InClientConnectionOpenCB)
{
    port = in_port;
    ClientConnectionOpenCB = InClientConnectionOpenCB;

    // Listen on given port
    CreateThread(nullptr, 0, [](LPVOID udp_server_instance) -> DWORD
    {
        eudp_server* server = static_cast<eudp_server*>(udp_server_instance);
        
        WSAData WsaData;
        if (WSAStartup(MAKEWORD(2, 2), &WsaData) != NO_ERROR)
        {
            LOG("Failed to start gameservers_handler: WSAStartup Failure.")
            thread_manager::suppress_thread("main");
            return 0;
        }

        // Initialize local address
        sockaddr_in server_address;
        ZeroMemory(&server_address, sizeof sockaddr_in);
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(server->port);
        server_address.sin_addr.s_addr = htonl(INADDR_ANY);

        // Initialize socket
        server->server_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (server->server_socket == SOCKET_ERROR)
        {
            LOG("Failed to initialize server_socket: SOCKET_ERROR.")
            thread_manager::suppress_thread("main");
            return 0;
        }

        // bind the socket to our address
        if (bind(server->server_socket, reinterpret_cast<sockaddr*>(&server_address), sizeof sockaddr_in) < 0)
        {
            LOG("Failed to bind server_socket.")
            thread_manager::suppress_thread("main");
            return 0;
        }

        // Initialize local client_address variable
        sockaddr_in client_address;
        int client_address_size = sizeof sockaddr_in;
        ZeroMemory(&client_address, client_address_size);

        // Local 'buffer' variable
        char Message[1024];
        
        // Read incoming messages with recvfrom
        while (true)
        {
            ZeroMemory(Message, 1024);
            const int MessageLength = recvfrom(server->server_socket, Message, 1024, 0, reinterpret_cast<sockaddr*>(&client_address), &client_address_size);

            if (MessageLength == 0)
            {
                break;
            }

            // Call message Handler
            HandleIncomingMessage(server, client_address, Message);
        }

        // Close socket
        closesocket(server->server_socket);

        return 0;
    }, this, 0, nullptr);
}

void eudp_server::HandleIncomingMessage(eudp_server* server, sockaddr_in client, char* Message)
{
    // If we don't know this client, try to auth them
    if (auto result_connection = std::find_if(server->connections.begin(), server->connections.end(), [&](econnection* x)
    {
        return x->network_addr.sin_addr.s_addr == client.sin_addr.s_addr && x->network_addr.sin_port == client.sin_port;
    }); result_connection == server->connections.end())
    {
        // Parse message, make sure this is an 'auth' message
        try
        {
            json message_formatted = json::parse(Message);

            try
            {
                if (message_formatted["type"].get<std::string>() == "auth")
                {
                    std::string key = message_formatted["payload"]["key"];

                    // Find a connection waiting for hello with this key
                    if (auto result = std::find_if(server->connections.begin(), server->connections.end(), [&](econnection* x)
                    {
                        return x->network_status == EConnectionNetworkStatus::WaitingForHello && x->key == key;
                    }); result != server->connections.end())
                    {
                        econnection* eligible_connection = *result;

                        eligible_connection->network_addr = client;

                        // Send a 'welcome' event
                        if (server->Send(eligible_connection, eudp_packet({
                            {"type", "welcome"},
                            {"payload", json()}
                        })))
                        {
                            eligible_connection->network_status = EConnectionNetworkStatus::WaitingForAck;
                        }
                    }
                }
            }
            catch (json::exception& err)
            {
            }
        }
        catch (json::parse_error& err)
        {
        }
    }
    else
    {
        // Parse message
        try
        {
            json message_formatted = json::parse(Message);

            try
            {
                std::string message_type = message_formatted["type"].get<std::string>();

                // Process different control messages
                if (message_type == "ack")
                {
                    if ((*result_connection)->network_status == EConnectionNetworkStatus::WaitingForAck)
                    {
                        (*result_connection)->network_status = EConnectionNetworkStatus::Open;
                        if (server->ClientConnectionOpenCB)
                        {
                            server->ClientConnectionOpenCB(server, *result_connection);
                        }
                        (*result_connection)->owner = server;

                        // Start heartbeat thread
                        CreateThread(nullptr, 0, [](LPVOID connection_instance)->DWORD
                        {
                            econnection* connection = static_cast<econnection*>(connection_instance);
                            connection->heartbeat_thread_id = "heartbeat:" + connection->key;

                            thread_manager::register_thread(GetCurrentThreadId(), connection->heartbeat_thread_id);

                            while (true)
                            {
                                if (thread_manager::is_thread_suppressed(GetCurrentThreadId()))
                                {
                                    thread_manager::unregister_thread(GetCurrentThreadId());
                                    break;
                                }
                                
                                connection->owner->Send(connection, eudp_packet({
                                    {"type","heartbeat"},
                                    {"payload", json()}
                                }));
                                connection->non_ack_heartbeats++;
                                Sleep(10000);

                                
                                if (connection->non_ack_heartbeats > 2)
                                {
                                    LOG("Connection '{}' did not acknowledge more than 30 seconds, closing connection [TIMED OUT].", connection->key.c_str());
                                    std::remove(connection->owner->connections.begin(), connection->owner->connections.end(), connection);
                    
                                    // callback for connection close
                                    if (connection->owner->ClientConnectionCloseCB)
                                    {
                                        connection->owner->ClientConnectionCloseCB(connection->owner, connection);
                                    }

                                    thread_manager::suppress_thread(connection->heartbeat_thread_id);
                                }
                            }
                            
                            return 0;
                        }, *result_connection, 0, nullptr);
                    }
                }
                else if (message_type == "heartbeat_ack")
                {
                    (*result_connection)->non_ack_heartbeats = 0;
                }
                else if (message_type == "close")
                {
                    // Send native close packet
                    server->Send(*result_connection, eudp_packet());
                
                    std::remove(server->connections.begin(), server->connections.end(), *result_connection);
                    
                    // callback for connection close
                    if (server->ClientConnectionCloseCB)
                    {
                        server->ClientConnectionCloseCB(server, *result_connection);
                    }

                    // Terminate the heartbeat thread
                    thread_manager::suppress_thread((*result_connection)->heartbeat_thread_id);
                }
                else
                {
                    for (auto& [key, val] : server->Events)
                    {
                        if (key == message_type && val.second == *result_connection)
                        {
                            val.first(server, *result_connection, message_formatted["payload"].get<json>());
                            break;
                        }
                    }
                    for (auto& [key, val] : server->OneTimeEvents)
                    {
                        if (key == message_type && val.second == *result_connection)
                        {
                            val.first(server, *result_connection, message_formatted["payload"].get<json>());
                            server->OneTimeEvents.erase(server->OneTimeEvents.find(key));
                            break;
                        }
                    }
                }
            }
            catch (json::exception& err)
            {
            }
        }
        catch (json::parse_error& err)
        {
        }
    }
}

bool eudp_server::Send(econnection* Client, eudp_packet Data)
{
    if (sendto(server_socket, Data.Payload.c_str(), Data.Size, 0, reinterpret_cast<sockaddr*>(&Client->network_addr), sizeof sockaddr_in) == SOCKET_ERROR)
    {
        LOG("Error while sending Message [{}] to Client [{}]", Data.Size, Client->key.c_str())
        return false;
    }
    
    return true;
}

void eudp_server::On(std::string EventName, econnection* connection, OnServerEventCB Callback)
{
    if (!Events.contains(EventName))
    {
        Events[EventName] = std::make_pair(Callback, connection);
    }
}

void eudp_server::WaitFor(std::string EventName, econnection* connection, OnServerEventCB Callback)
{
    if (!OneTimeEvents.contains(EventName))
    {
        OneTimeEvents[EventName] = std::make_pair(Callback, connection);
    }
}

void eudp_server::OnClose(OnClientConnectionClose CloseCB)
{
    ClientConnectionCloseCB = CloseCB;
}

eudp_client::eudp_client(std::string in_addr, int in_port, std::string in_key, OnServerConnectionOpen InServerConnectionOpenCB)
{
    addr = in_addr;
    port = in_port;
    key = in_key;
    network_status = EClientNetworkStatus::WaitingForWelcome;
    ServerConnectionOpenCB = InServerConnectionOpenCB;

    CreateThread(nullptr, 0, [](LPVOID udp_client_instance) -> DWORD
    {
        eudp_client* client = static_cast<eudp_client*>(udp_client_instance);
        
        WSAData WsaData;
        if (WSAStartup(MAKEWORD(2, 2), &WsaData) != NO_ERROR)
        {
            LOG("Failed to start gameservers_handler: WSAStartup Failure.")
            thread_manager::suppress_thread("main");
            return 0;
        }

        // Initialize local address
        ZeroMemory(&client->server_addr, sizeof sockaddr_in);
        client->server_addr.sin_family = AF_INET;
        client->server_addr.sin_port = htons(client->port);
        client->server_addr.sin_addr.s_addr = inet_addr(client->addr.c_str());
        int server_addr_len = sizeof sockaddr_in;

        // Initialize socket
        client->client_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (client->client_socket == SOCKET_ERROR)
        {
            LOG("Failed to initialize server_socket: SOCKET_ERROR.")
            thread_manager::suppress_thread("main");
            return 0;
        }
        
        // Local 'buffer' variable
        char Message[65535];

        // Send key to server
        client->Send(eudp_packet({
            {"type", "auth"},
            {"payload", {
                {"key", client->key}
            }}
        }));
        
        // Read incoming messages with recvfrom
        while (true)
        {
            ZeroMemory(Message, 65535);
            const int MessageLength = recvfrom(client->client_socket, Message, 65535, 0, reinterpret_cast<sockaddr*>(&client->server_addr), &server_addr_len);

            if (MessageLength == 0)
            {
                LOG("Client terminated, received message with length 0. Error. {}", WSAGetLastError());
                break;
            }

            // Call message Handler
            HandleIncomingMessage(client, Message);
        }

        // Close socket
        closesocket(client->client_socket);
        
        return 0;
    }, this, 0, nullptr);
}

void eudp_client::HandleIncomingMessage(eudp_client* client, char* Message)
{
    // Parse message
    try
    {
        json message_formatted = json::parse(Message);

        try
        {
            std::string type = message_formatted["type"].get<std::string>();

            if (type == "welcome")
            {
                client->Send(eudp_packet({
                    {"type", "ack"},
                    {"payload", json()}
                }));

                client->network_status = EClientNetworkStatus::Open;

                if (client->ServerConnectionOpenCB)
                {
                    client->ServerConnectionOpenCB(client);
                }
            }
            else if (type == "heartbeat")
            {
                client->Send(eudp_packet({
                    {"type", "heartbeat_ack"},
                    {"payload", json()}
                }));
            }
            else
            {
                for (auto& [key, val] : client->Events)
                {
                    if (key == type)
                    {
                        val(client, message_formatted["payload"].get<json>());
                        break;
                    }
                }
                for (auto& [key, val] : client->OneTimeEvents)
                {
                    if (key == type)
                    {
                        val(client, message_formatted["payload"].get<json>());
                        client->OneTimeEvents.erase(client->OneTimeEvents.find(key));
                        break;
                    }
                }
            }
        }
        catch (json::exception& err)
        {
        }
    }
    catch(json::parse_error& err)
    {
    }
}

bool eudp_client::Send(eudp_packet Data)
{
    if (sendto(client_socket, Data.Payload.c_str(), Data.Size, 0, reinterpret_cast<sockaddr*>(&server_addr), sizeof sockaddr_in) == SOCKET_ERROR)
    {
        LOG("Error while sending Message [{}] to Server.", Data.Size)
        return false;
    }
    
    return true;
}

void eudp_client::On(std::string EventName, OnClientEventCB Callback)
{
    if (!Events.contains(EventName))
    {
        Events[EventName] = Callback;
    }
}

void eudp_client::WaitFor(std::string EventName, OnClientEventCB Callback)
{
    if (!OneTimeEvents.contains(EventName))
    {
        OneTimeEvents[EventName] = Callback;
    }
}

void eudp_client::Close()
{
    Send(eudp_packet({
        {"type", "close"},
        {"payload", json()}
    }));
}

