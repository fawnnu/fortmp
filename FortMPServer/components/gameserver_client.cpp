#include "include.hpp"
#include "gameserver_client.hpp"

#include "challenges_component.hpp"
#include "config_manager.hpp"
#include "director_client.hpp"
#include "era_mutators_component.hpp"
#include "loader_component.hpp"
#include "log_component.hpp"
#include "net_component.hpp"
#include "player_component.hpp"
#include "qos_component.hpp"
#include "team_component.hpp"
#include "../gui/renderer.hpp"
#include "../utils/webhook_manager.hpp"

void HeartbeatThread()
{
    while (true)
    {
        Sleep(10000);

        if (gameserver_client::Client->LocalNonAckHeartbeats > 20)
        {
            // Log
            spdlog::error(L"Server has not responded to more than 20 heartbeats, closing...");
            
            // Unregister the server
            gameserver_client::Client->SendLeave();

            // Close director connection
            if (director_client::client)
            {
                director_client::client->Close();
            }
            
            // Request exit if the server has not been responsive for more than 3 minutes
            RequestExitWithStatus(true, 0);
        }
        else if (gameserver_client::Client->LocalNonAckHeartbeatsForPlayers > 30)
        {
            // Log
            spdlog::error(L"Server has had no players for more than 30 heartbeats, closing...");
            
            // Unregister the server
            gameserver_client::Client->SendLeave();
            
            // Close director connection
            if (director_client::client)
            {
                director_client::client->Close();
            }
            
            // Request exit if the server has not been responsive for more than 5 minutes
            RequestExitWithStatus(true, 0);
        }
        else
        {
            gameserver_client::Client->LocalNonAckHeartbeats++;

            if (*GWorld && (*GWorld)->NetDriver())
            {
                gameserver_client::Client->LocalNonAckHeartbeatsForPlayers++;
            }
        }
        
        websocketpp::lib::error_code ec;
        auto Connection = gameserver_client::Client->Client_WS.get_con_from_hdl(gameserver_client::Client->Connection_Handle, ec);

        if (Connection)
        {
            if (Connection->get_state() == websocketpp::session::state::open)
            {
                gameserver_client::Client->SendHeartbeat();
                continue;
            }
        }
        
        {
            // Log
            spdlog::warn(L"Lost connection to Era Services, attempting reconnection...");

            // Restart server as soon as possible
            loader_component::bPendingShutdown = true;
            
            websocketpp::lib::error_code error_code;
            Connection = gameserver_client::Client->Client_WS.get_connection(WS_URL + "/internal/game/websocket?token=bn8HUsAfhogEq6E57ledvyGReK9n8E7XP53AbRomzvItAexNB09aXiTgFhoPvUD1&vsn=2.0.0", error_code);
    
            if (error_code)
            {
                spdlog::error("Failed to re-initialize GameServer Client with error code: {}. Era Services will not see this server.", error_code.message().c_str());
                return;
            }

            gameserver_client::Client->Connection_Handle = Connection->get_handle();

            gameserver_client::Client->Client_WS.connect(Connection);
            
            gameserver_client::Client->Client_WS.stop();
        }
    }
}

gameserver_client::gameserver_client()
{
#ifdef _RELEASE
    // Initialize self
    Client = this;
    
    // Initialize status
    GameServer_Status = gameserver_status();
#endif
}

void gameserver_client::Initialize()
{
#ifdef _RELEASE
    {
        // Initialize our client object
        Aws::Client::ClientConfiguration ClientConfig;
        ClientConfig.endpointOverride = "34e19315f44dab48bc0d496bea16a1d0.r2.cloudflarestorage.com"; 
        Aws::Auth::AWSCredentials Credentials("a6dc961104a779468543630be4e6259b", "f989383ce3f277383fc3ff7fa6845438ff6c92faac0629dafa4aab1ea0dbee64");
        S3Client = std::make_shared<Aws::S3::S3Client>(Credentials, Aws::MakeShared<Aws::S3::S3EndpointProvider>(Aws::S3::S3Client::ALLOCATION_TAG), ClientConfig);
    }
    
    // initialize the client
    Client_WS.init_asio();

    // Set handlers
    Client_WS.set_open_handler(websocketpp::lib::bind(&gameserver_client::OnConnected, this, websocketpp::lib::placeholders::_1));
    
    Client_WS.set_message_handler(websocketpp::lib::bind(&gameserver_client::OnMessageReceived, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
    
    Client_WS.set_tls_init_handler([this](websocketpp::connection_hdl)
    {
        return websocketpp::lib::make_shared<asio::ssl::context>(asio::ssl::context::tls_client);
    });
    
    websocketpp::lib::error_code error_code;
    auto Connection = Client_WS.get_connection(WS_URL + "/internal/game/websocket?token=bn8HUsAfhogEq6E57ledvyGReK9n8E7XP53AbRomzvItAexNB09aXiTgFhoPvUD1&vsn=2.0.0", error_code);
    
    if (error_code)
    {
        spdlog::error("Failed to initialize GameServer Client with error code: {}. Era Services will not see this server.", error_code.message().c_str());
        return;
    }

    Connection_Handle = Connection->get_handle();
    
    Client_WS.connect(Connection);

    // Start the scheduler thread
    CreateThread(0, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(scheduler_client::Initialize), nullptr, 0, nullptr);
    
    // Start the heartbeat thread
    CreateThread(0,0, reinterpret_cast<LPTHREAD_START_ROUTINE>(&HeartbeatThread), 0,0,0);
    
    while (true)
    {
        Client_WS.run();

        Sleep(1000);
    }
#endif
}

void gameserver_client::PreInitialize()
{
    const Aws::SDKOptions Options;
    InitAPI(Options);
}

void gameserver_client::Register()
{
    bool bSucceeded = false;
    while (!bSucceeded)
    {
        nlohmann::json server_config = config_manager::get_server_config();
        GameServer_Status.max_players = static_cast<int>(server_config["player_cap_override"]);
    
        if (server_config.contains("server_key"))
        {
            std::string server_key = server_config["server_key"];
            if (server_key.length() >= 4)
            {
                GameServer_Status.private_matchmaking_key = server_key;
            }
        }

        httplib::Client client(API_URL);
        httplib::Headers headers = {
            { "x-era-token", "bn8HUsAfhogEq6E57ledvyGReK9n8E7XP53AbRomzvItAexNB09aXiTgFhoPvUD1" }
        };
        nlohmann::json PostPayload = {
            {"region", ""},
            {"build", std::to_string(GetCL())},
            {"max_players", std::to_string(GameServer_Status.max_players)},
            {"key", GameServer_Status.private_matchmaking_key}
        };

        // Get grants from config
        if (server_config.contains("grants") && server_config["grants"].is_array())
        {
            nlohmann::json grants_array = server_config["grants"];
            for (auto& elem : grants_array.items())
            {
                PostPayload["grants"].push_back(elem.value());
            }
        }
    
        PostPayload["region"] = qos_component::best_region;
        client.set_read_timeout(15, 0); // 15 seconds
        client.set_write_timeout(15, 0); // 15 seconds
        auto Result = client.Post("/api/internal/game/servers", headers, PostPayload.dump(), "application/json");
    
        if (Result && Result->status == 200)
        {
            try
            {
                nlohmann::json JsonResult = nlohmann::json::parse(Result->body);

                if (JsonResult.contains("data"))
                {
                    bSucceeded = true;
                    UUID = std::string(JsonResult["data"]["uuid"]);

                    if (server_config.contains("name"))
                    {
                        std::string server_name = server_config["name"];
                        if (server_name.length() >= 2 && server_name.length() <= 16)
                        {
                            GameServer_Status.name = server_name;
                            if (renderer::WindowHandle)
                            {
                                SetWindowTextA(renderer::WindowHandle, std::string("FortMP - " + Client->GameServer_Status.name).c_str());
                            }
                            else
                            {
                                SetConsoleTitleA(std::string("FortMP - " + Client->GameServer_Status.name).c_str());
                            }
                        }
                    }
    
                    SendJoin();
                    SendUpdate();

                    // Get cube info
                    try
                    {
                        for (auto& item : JsonResult["data"]["event_states"].items())
                        {
                            std::string type = item.value()["type"];
                            if (type == "cube")
                            {
                                int tracking = item.value()["state"]["tracking"];
                                cube_mutator::step = tracking;
                            }
                        }
                    }
                    catch (nlohmann::json::exception& err)
                    {
                        spdlog::error("gameserver_client::Register() caught error while trying to read kevin data: {}", err.what());
                    }

                    break;
                }
                else
                {
                    spdlog::error("POST /api/internal/game/servers returned {}. Body: {}", Result->status, Result->body.c_str());
                }
            }
            catch (nlohmann::json::parse_error& ex)
            {
        
            }
        }
        else
        {
            spdlog::error("/api/internal/game/servers failed. Retrying in 100ms");
            if (Result)
            {
                spdlog::error("Error code for /api/internal/game/servers: '{}'", Result->status);
            }
        }
        
        Sleep(100);
    }
}

void gameserver_client::WaitForReply(int MessageReference, PVOID callback)
{
    if (MessageReference >= 0)
    {
        if (Client)
        {
            if (!Client->MessagesMap.contains(MessageReference))
            {
                Client->MessagesMap[MessageReference] = callback;
            }
        }
    }
}

void gameserver_client::OnConnected(websocketpp::connection_hdl handle)
{
#ifdef _RELEASE
    Register();
#endif
}

void gameserver_client::OnMessageReceived(websocketpp::connection_hdl handle, websocketpp::config::asio_client::message_type::ptr message)
{
#ifdef _RELEASE
    auto payload = message->get_payload();

    try
    {
        auto JsonPayload = nlohmann::json::parse(payload);
        
        std::vector<std::string> VectorPayloadString;
        std::vector<int> VectorPayloadInt;
        nlohmann::json JsonPayloadData;
        for (auto& elem : JsonPayload.items())
        {
            if (elem.value().is_number_integer())
            {
                VectorPayloadInt.push_back(elem.value());
            }
            
            if (elem.value().is_string())
            {
                VectorPayloadString.push_back(elem.value());
            }

            if (elem.value().is_object())
            {
                JsonPayloadData = elem.value();
            }
        }

        // Make sure it's a phoenix message
        try
        {
            if (VectorPayloadString.size() == 2)
            {
                if (VectorPayloadString[1] == "event:start")
                {
                    if (JsonPayloadData.contains("type") && JsonPayloadData["type"].is_string())
                    {
                        live_events.push_back(JsonPayloadData["type"].get<std::string>());
                    }
                }
            
                // If it's a phx_error message, log it
                if (VectorPayloadString[1] == "phx_error")
                {
                    std::string ChannelName = VectorPayloadString[0];
                    if (ChannelName.size() > 0)
                    {
                        spdlog::error("Channel '{}' crashed. [Received phx_error]", ChannelName.c_str());
                        try
                        {
                            spdlog::error("phx_error last message: {}", LastMessagePayload.dump().c_str());
                        }
                        catch (nlohmann::json::other_error& err)
                        {
                            spdlog::error("phx_error last message can't be dumped.");
                        }
                    }
                }
            
                // If it's a phx_reply message, check if there's any callback to call
                if (VectorPayloadString[1] == "phx_reply")
                {
                    if (MessagesMap.contains(VectorPayloadInt[1]))
                    {
                        // Send the event to be processed by gamethread
                        event_replies.push_back(new reply_info(MessagesMap[VectorPayloadInt[1]], VectorPayloadString[0], JsonPayloadData));

                        // Remove message from MessagesMap
                        MessagesMap.erase(MessagesMap.find(VectorPayloadInt[1]));
                    }
                }
            
                // If it's a "progression" message, process it to send the client a live progression notification
                if (VectorPayloadString[1] == "progression")
                {
                    if (JsonPayloadData.contains("challenge") &&
                        JsonPayloadData.contains("objective") &&
                        JsonPayloadData.contains("value"))
                    {
                        std::string challenge = JsonPayloadData["challenge"];
                        std::string objective = JsonPayloadData["objective"];
                        std::string assisted_by = JsonPayloadData["meta"]["assisted_by"];
                        bool assisted = JsonPayloadData["meta"]["assisted"];
                        int value = JsonPayloadData["value"];

                        std::string ChannelName = VectorPayloadString[0];

                        if (ChannelName.find("player:v2:") != std::string::npos)
                        {
                            std::string AccountID = ChannelName.substr(ChannelName.find(":v2:") + 4);

                            progression_entry* new_entry = new progression_entry;
                            new_entry->challenge = challenge;
                            new_entry->objective = objective;
                            new_entry->value = value;
                            new_entry->player = AccountID;
                            new_entry->meta.assisted = assisted;
                            new_entry->meta.assisted_by = assisted_by;
                            new_entry->delta = 0;
                            try
                            {
                                new_entry->delta = JsonPayloadData["delta"];
                            }
                            catch (nlohmann::json::exception& err)
                            {
                            }

                            challenges_progression_queue.push_back(new_entry);
                        }
                    }
                }
        
                // If we receive a kick message, kick the target player
                if (VectorPayloadString[1] == "kick")
                {
                    kick_events_queue.push_back(JsonPayloadData);
                }
        
                // If it's a "player" message, add the encryptiontoken to the encryptiontoken list
                if (VectorPayloadString[1] == "player")
                {
                    std::string sEncryptionToken = std::string(JsonPayloadData["account_id"]) + ":" + std::string(JsonPayloadData["session_id"]);
                    std::wstring wEncryptionToken = std::wstring(sEncryptionToken.begin(), sEncryptionToken.end());
           
                    // Add to the valid EncryptionToken list
                    player_component::EncryptionTokenList.push_back(wEncryptionToken);

                    // Properly handle incoming fill and ticket
                    if (JsonPayloadData.contains("fill") && JsonPayloadData.contains("ticket"))
                    {
                        bool fill = false;
                        std::string ticket = "";
                
                        if (JsonPayloadData["ticket"].is_string())
                        {
                            ticket = JsonPayloadData["ticket"];
                        }
                        if (JsonPayloadData["fill"].is_boolean())
                        {
                            fill = JsonPayloadData["fill"];
                        }

                        if (ticket.length() > 0)
                        {
                            const std::wstring wTicket = std::wstring(ticket.begin(), ticket.end());
                    
                            auto& new_team_instance = team_component::TeamsMap[wTicket];
                            new_team_instance.TeamMembers.push_back(wEncryptionToken);
                            new_team_instance.fill = fill;
                        }
                    }

                    if (JsonPayloadData.contains("profile"))
                    {
                        if (JsonPayloadData["profile"].is_object())
                        {
                            //&/player_component::Profiles[wEncryptionToken] = JsonPayloadData["profile"];
                        }
                    }
                }
            }
        }
        catch (std::exception& err)
        {
            spdlog::error("gameserver_client::OnMessageReceived: failed to process incoming message. Error: {}.", err.what());
        }
    }
    catch (nlohmann::json::parse_error& ex)
    {
    }
#endif
}

int gameserver_client::SendJoin()
{
#ifdef _RELEASE
    websocketpp::lib::error_code ec;
    auto Connection = Client->Client_WS.get_con_from_hdl(Client->Connection_Handle, ec);

    if (Connection)
    {
        if (Connection->get_state() == websocketpp::session::state::open)
        {
            nlohmann::json payload = R"([])"_json;

            // Copy the message before bumping it
            const int CurrentMessageReference = MessageRef;

            payload.push_back(0); // Channel Ref
            payload.push_back(MessageRef++); // Message ref
            payload.push_back("server:" + UUID); // Channel
            payload.push_back("phx_join"); // Event Type
            payload.push_back(std::map<std::string, std::string>{
                {"name", {GameServer_Status.name.length() >= 2 && GameServer_Status.name.length() <= 16 ? GameServer_Status.name : qos_component::best_region}}
            }); // param

            // Copy last message
            LastMessagePayload = payload;
            
            // Send
            Client_WS.send(Connection_Handle, payload.dump(), websocketpp::frame::opcode::text);

            return CurrentMessageReference;
        }
    }

    return -1;
#endif
}

int gameserver_client::SendUpdate()
{
#ifdef _RELEASE
    websocketpp::lib::error_code ec;
    auto Connection = Client->Client_WS.get_con_from_hdl(Client->Connection_Handle, ec);

    if (Connection)
    {
        if (Connection->get_state() == websocketpp::session::state::open)
        {
            nlohmann::json payload = R"([])"_json;
            
            // Copy the message before bumping it
            const int CurrentMessageReference = MessageRef;

            payload.push_back(0); // Channel Ref
            payload.push_back(MessageRef++); // Message ref
            payload.push_back("server:" + UUID); // Channel
            payload.push_back("update"); // Event Type
            payload.push_back(std::map<std::string, std::string>{
                {"accepting_players", GameServer_Status.accepting_players ? "true" : "false"},
                {"number_of_players", std::to_string(GameServer_Status.number_of_players)},
                {"status", GameServer_Status.status},
                {"max_players", std::to_string(GameServer_Status.max_players)},
                {"port", std::to_string(GameServer_Status.port)},
                {"name", {GameServer_Status.name.length() >= 2 && GameServer_Status.name.length() <= 16 ? GameServer_Status.name : qos_component::best_region}},
                {"playlist", {GameServer_Status.mode}}
            }); // param

            // Copy last message
            LastMessagePayload = payload;
            
            // Send
            Client_WS.send(Connection_Handle, payload.dump(), websocketpp::frame::opcode::text);

            return CurrentMessageReference;
        }
    }

    return -1;
#endif
}

int gameserver_client::SendLeave()
{
#ifdef _RELEASE
    websocketpp::lib::error_code ec;
    auto Connection = Client->Client_WS.get_con_from_hdl(Client->Connection_Handle, ec);

    if (Connection)
    {
        if (Connection->get_state() == websocketpp::session::state::open)
        {
            nlohmann::json payload = R"([])"_json;

            // Copy the message before bumping it
            const int CurrentMessageReference = MessageRef;

            payload.push_back(0); // Channel Ref
            payload.push_back(MessageRef++); // Message ref
            payload.push_back("server:" + UUID); // Channel
            payload.push_back("phx_leave"); // Event Type
            payload.push_back(std::map<std::string, std::string>{
            }); // param

            // Copy last message
            LastMessagePayload = payload;
            
            // Send
            Client_WS.send(Connection_Handle, payload.dump(), websocketpp::frame::opcode::text);

            return CurrentMessageReference;
        }
    }

#endif
    return -1;
}

int gameserver_client::SendBan(std::string uuid)
{
#ifdef _RELEASE
    websocketpp::lib::error_code ec;
    auto Connection = Client->Client_WS.get_con_from_hdl(Client->Connection_Handle, ec);

    if (Connection)
    {
        if (Connection->get_state() == websocketpp::session::state::open)
        {
            nlohmann::json payload = R"([])"_json;

            // Copy the message before bumping it
            const int CurrentMessageReference = MessageRef;

            payload.push_back(0); // Channel Ref
            payload.push_back(MessageRef++); // Message ref
            payload.push_back("server:" + UUID); // Channel
            payload.push_back("ban:hwid"); // Event Type
            payload.push_back(std::map<std::string, std::string>{
                {"account_id", uuid}
            }); // param
            
            // Copy last message
            LastMessagePayload = payload;
            
            // Send
            Client_WS.send(Connection_Handle, payload.dump(), websocketpp::frame::opcode::text);

            return CurrentMessageReference;
        }
    }

#endif
    return -1;
}

int gameserver_client::SendHeartbeat()
{
#ifdef _RELEASE
    websocketpp::lib::error_code ec;
    auto Connection = Client->Client_WS.get_con_from_hdl(Client->Connection_Handle, ec);

    if (Connection)
    {
        if (Connection->get_state() == websocketpp::session::state::open)
        {
            nlohmann::json payload = R"([])"_json;

            // Copy the message before bumping it
            const int CurrentMessageReference = MessageRef;

            payload.push_back(0); // Channel Ref
            payload.push_back(MessageRef++); // Message ref
            payload.push_back("phoenix"); // Channel
            payload.push_back("heartbeat"); // Event Type
            payload.push_back(std::map<std::string, std::string>{
            }); // param

            // Copy last message
            LastMessagePayload = payload;
            
            // Send
            Client_WS.send(Connection_Handle, payload.dump(), websocketpp::frame::opcode::text);

            return CurrentMessageReference;
        }
    }

    return -1;
#endif
}

int gameserver_client::SendEventToChannel(std::string channel, std::string event_type, std::map<std::string, std::string> parameters)
{
#ifdef _RELEASE
    websocketpp::lib::error_code ec;
    auto Connection = Client->Client_WS.get_con_from_hdl(Client->Connection_Handle, ec);

    if (Connection)
    {
        if (Connection->get_state() == websocketpp::session::state::open)
        {
            nlohmann::json payload = R"([])"_json;

            // Copy the message before bumping it
            const int CurrentMessageReference = MessageRef;

            payload.push_back(0); // Channel Ref
            payload.push_back(MessageRef++); // Message ref
            payload.push_back(channel); // Channel
            payload.push_back(event_type); // Event Type
            payload.push_back(parameters); // param
            
            // Copy last message
            LastMessagePayload = payload;
            
            // Send
            websocketpp::lib::error_code ec2;
            Client_WS.send(Connection_Handle, payload.dump(), websocketpp::frame::opcode::text, ec2);

            if (log_component::enable_debug_logs)
            {
                if (ec2)
                {
                    spdlog::error("gameserver_client::SendEventToChannel error: {} [event: {}, channel: {}]", ec2.message().c_str(), event_type.c_str(), channel.c_str());
                }
                else
                {
                    spdlog::info("Successfully sent {} in {}", event_type.c_str(), channel.c_str());
                }
            }

            return CurrentMessageReference;
        }
    }

#endif
    return -1;
}

int gameserver_client::SendEventToChannel(std::string channel, std::string event_type, nlohmann::json parameters)
{
#ifdef _RELEASE
    websocketpp::lib::error_code ec;
    auto Connection = Client->Client_WS.get_con_from_hdl(Client->Connection_Handle, ec);

    if (Connection)
    {
        if (Connection->get_state() == websocketpp::session::state::open)
        {
            nlohmann::json payload = R"([])"_json;

            // Copy the message before bumping it
            const int CurrentMessageReference = MessageRef;

            payload.push_back(0); // Channel Ref
            payload.push_back(MessageRef++); // Message ref
            payload.push_back(channel); // Channel
            payload.push_back(event_type); // Event Type
            payload.push_back(parameters); // param

            // Copy last message
            LastMessagePayload = payload;
            
            // Send
            websocketpp::lib::error_code ec2;
            Client_WS.send(Connection_Handle, payload.dump(), websocketpp::frame::opcode::text, ec2);

            if (log_component::enable_debug_logs)
            {
                if (ec2)
                {
                    spdlog::error("gameserver_client::SendEventToChannel error: {} [event: {}, channel: {}]", ec2.message().c_str(), event_type.c_str(), channel.c_str());
                }
                else
                {
                    spdlog::info("Successfully sent {} in {}", event_type.c_str(), channel.c_str());
                }
            }

            return CurrentMessageReference;
        }
    }

#endif
    return -1;
}

void scheduler_client::Initialize()
{
    auto config = config_manager::get_server_config();

    Port = config["scheduler_port"];
    
    if (Port == -1)
    {
        // Invalid port / not meant to use scheduler, abort setup
        spdlog::warn(L"Failed to initialize scheduler client: provided port ({}) is invalid.", Port);

        return;
    }

    spdlog::info(L"Initializing scheduler client. Server port: {}", Port);

    // Start a client and register to scheduler server
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
    {
        spdlog::error(L"Failed to initialize scheduler client.");
        
        return; 
    }
    
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == SOCKET_ERROR)
    {
        spdlog::error(L"Failed to initialize scheduler client. Socket creation failed.");
        
        return;
    }

    // Server info
    sockaddr_in server_address;
    ZeroMemory(&server_address, sizeof server_address);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(Port);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    int server_address_len = sizeof server_address;

    // Register
    char send_buffer[1];
    send_buffer[0] = 0x1;
    char recv_buffer[1024];

    if (sendto(sock, send_buffer, sizeof send_buffer, 0, reinterpret_cast<sockaddr*>(&server_address), sizeof server_address) == SOCKET_ERROR)
    {
        spdlog::error(L"Scheduler client: failed to send register packet.");
        
        return;
    }

    // Handle incoming messages
    while (true)
    {
        ZeroMemory(recv_buffer, sizeof recv_buffer);
        
        int len = recvfrom(sock, recv_buffer, sizeof recv_buffer, 0, reinterpret_cast<sockaddr*>(&server_address), &server_address_len);

        if (len == SOCKET_ERROR)
        {
            spdlog::error(L"Scheduler client: recvfrom failed. Last error: {}", WSAGetLastError());
        }
        
        // Valid length
        if (len > 0)
        {
            // Request exit
            if (recv_buffer[0] == 0x2)
            {
                // Log
                spdlog::warn(L"Server closing gracefully (requested by scheduler).");
                
                // Mark as pending shutdown
                loader_component::bPendingShutdown = true;
            }
        }
    }
}