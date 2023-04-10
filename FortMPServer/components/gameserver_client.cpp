#include "include.hpp"
#include "gameserver_client.hpp"

#include "config_manager.hpp"
#include "net_component.hpp"
#include "player_component.hpp"
#include "qos_component.hpp"
#include "team_component.hpp"
#include "../gui/renderer.hpp"

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
            
            // Request exit if the server has not been responsive for more than 3 minutes
            RequestExitWithStatus(true, 0);
        }
        else if (gameserver_client::Client->LocalNonAckHeartbeatsForPlayers > 30)
        {
            // Log
            spdlog::error(L"Server has had no players for more than 30 heartbeats, closing...");
            
            // Unregister the server
            gameserver_client::Client->SendLeave();
            
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

    // Start the heartbeat thread
    CreateThread(0,0, reinterpret_cast<LPTHREAD_START_ROUTINE>(&HeartbeatThread), 0,0,0);
    
    while (true)
    {
        Client_WS.run();

        Sleep(1000);
    }
#endif
}

void gameserver_client::Register()
{
    nlohmann::json server_config = config_manager::get_server_config();
    GameServer_Status.max_players = static_cast<int>(server_config["player_cap_override"]);

    httplib::Client client(API_URL);
    httplib::Headers headers = {
        { "x-era-token", "bn8HUsAfhogEq6E57ledvyGReK9n8E7XP53AbRomzvItAexNB09aXiTgFhoPvUD1" }
    };
    nlohmann::json PostPayload = {
        {"region", ""},
        {"build", std::to_string(GetCL())},
        {"max_players", std::to_string(GameServer_Status.max_players)}
    };
    PostPayload["region"] = qos_component::best_region;
    auto Result = client.Post("/api/internal/game/servers", headers, PostPayload.dump(), "application/json");
    nlohmann::json JsonResult = nlohmann::json::parse(Result->body);

    Channel = "server:" + std::string(JsonResult["data"]["uuid"]);

    // Parse private_matchmaking_key
    if (server_config.contains("server_key"))
    {
        std::string server_key = server_config["server_key"];
        if (server_key.length() >= 4)
        {
            GameServer_Status.private_matchmaking_key = server_key;
        }
    }
    if (server_config.contains("name"))
    {
        std::string server_name = server_config["name"];
        if (server_name.length() >= 2 && server_name.length() <= 16)
        {
            GameServer_Status.name = server_name;
            SetWindowTextA(renderer::WindowHandle, std::string("FortMP - " + Client->GameServer_Status.name).c_str());
        }
    }
    
    SendJoin();
    SendUpdate();   
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

    auto JsonPayload = nlohmann::json::parse(payload);

    std::vector<std::string> VectorPayloadString;
    nlohmann::json JsonPayloadData;
    for (auto& elem : JsonPayload.items())
    {
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
    if (VectorPayloadString.size() == 2)
    {
        // If we receive a kick message, kick the target player
        if (VectorPayloadString[1] == "kick")
        {
            if (JsonPayloadData.contains("account_id"))
            {
                std::string sAccountID = JsonPayloadData["account_id"];

                if (sAccountID.size())
                {
                    std::wstring wAccountID = std::wstring(sAccountID.begin(), sAccountID.end());

                    if (wAccountID.size())
                    {
                        for (auto [account_id, player] : player_component::Players)
                        {
                            if (account_id == wAccountID)
                            {
                                if (!memory_manager::is_bad_read_pointer(player) && player)
                                {
                                    if (!memory_manager::is_bad_read_pointer(player->MyPlayerInfo()) && player->MyPlayerInfo())
                                    {
                                        spdlog::info(L"Kicked '{}' [account_id: {}] [KICK EVENT]", player->MyPlayerInfo()->PlayerName().c_str(), wAccountID.c_str());
                                    }
                                    
                                    player->ClientReturnToMainMenu(TEXT(""));

                                    break;
                                }
                            }
                        }
                    }
                }
            }
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
                    player_component::Profiles[wEncryptionToken] = JsonPayloadData["profile"];
                }
            }
        }
    }
#endif
}

void gameserver_client::SendJoin()
{
#ifdef _RELEASE
    websocketpp::lib::error_code ec;
    auto Connection = Client->Client_WS.get_con_from_hdl(Client->Connection_Handle, ec);

    if (Connection)
    {
        if (Connection->get_state() == websocketpp::session::state::open)
        {
            nlohmann::json payload = R"([])"_json;

            payload.push_back(0); // Channel Ref
            payload.push_back(MessageRef++); // Message ref
            payload.push_back(Channel); // Channel
            payload.push_back("phx_join"); // Event Type
            payload.push_back(std::map<std::string, std::string>{
                {"private_matchmaking_key", {GameServer_Status.private_matchmaking_key.length() >= 4 ? GameServer_Status.private_matchmaking_key : ""}},
                {"name", {GameServer_Status.name.length() >= 2 && GameServer_Status.name.length() <= 16 ? GameServer_Status.name : qos_component::best_region}}
            }); // param

            // Send
            Client_WS.send(Connection_Handle, payload.dump(), websocketpp::frame::opcode::text);
        }
    }
#endif
}

void gameserver_client::SendUpdate()
{
#ifdef _RELEASE
    websocketpp::lib::error_code ec;
    auto Connection = Client->Client_WS.get_con_from_hdl(Client->Connection_Handle, ec);

    if (Connection)
    {
        if (Connection->get_state() == websocketpp::session::state::open)
        {
            nlohmann::json payload = R"([])"_json;

            payload.push_back(0); // Channel Ref
            payload.push_back(MessageRef++); // Message ref
            payload.push_back(Channel); // Channel
            payload.push_back("update"); // Event Type
            payload.push_back(std::map<std::string, std::string>{
                {"accepting_players", GameServer_Status.accepting_players ? "true" : "false"},
                {"number_of_players", std::to_string(GameServer_Status.number_of_players)},
                {"status", GameServer_Status.status},
                {"max_players", std::to_string(GameServer_Status.max_players)},
                {"private_matchmaking_key", {GameServer_Status.private_matchmaking_key.length() >= 4 ? GameServer_Status.private_matchmaking_key : ""}},
                {"port", std::to_string(GameServer_Status.port)},
                {"name", {GameServer_Status.name.length() >= 2 && GameServer_Status.name.length() <= 16 ? GameServer_Status.name : qos_component::best_region}},
                {"playlist", {GameServer_Status.mode}}
            }); // param

            // Send
            Client_WS.send(Connection_Handle, payload.dump(), websocketpp::frame::opcode::text);
        }
    }
#endif
}

void gameserver_client::SendLeave()
{
#ifdef _RELEASE
    websocketpp::lib::error_code ec;
    auto Connection = Client->Client_WS.get_con_from_hdl(Client->Connection_Handle, ec);

    if (Connection)
    {
        if (Connection->get_state() == websocketpp::session::state::open)
        {
            nlohmann::json payload = R"([])"_json;

            payload.push_back(0); // Channel Ref
            payload.push_back(MessageRef++); // Message ref
            payload.push_back(Channel); // Channel
            payload.push_back("phx_leave"); // Event Type
            payload.push_back(std::map<std::string, std::string>{
            }); // param

            // Send
            Client_WS.send(Connection_Handle, payload.dump(), websocketpp::frame::opcode::text);
        }
    }
#endif
}

void gameserver_client::SendBan(std::string uuid)
{
#ifdef _RELEASE
    websocketpp::lib::error_code ec;
    auto Connection = Client->Client_WS.get_con_from_hdl(Client->Connection_Handle, ec);

    if (Connection)
    {
        if (Connection->get_state() == websocketpp::session::state::open)
        {
            nlohmann::json payload = R"([])"_json;

            payload.push_back(0); // Channel Ref
            payload.push_back(MessageRef++); // Message ref
            payload.push_back(Channel); // Channel
            payload.push_back("ban:hwid"); // Event Type
            payload.push_back(std::map<std::string, std::string>{
                {"account_id", uuid}
            }); // param

            // Send
            Client_WS.send(Connection_Handle, payload.dump(), websocketpp::frame::opcode::text);
        }
    }
#endif
}

void gameserver_client::SendHeartbeat()
{
#ifdef _RELEASE
    websocketpp::lib::error_code ec;
    auto Connection = Client->Client_WS.get_con_from_hdl(Client->Connection_Handle, ec);

    if (Connection)
    {
        if (Connection->get_state() == websocketpp::session::state::open)
        {
            nlohmann::json payload = R"([])"_json;

            payload.push_back(nullptr); // Channel Ref
            payload.push_back(MessageRef++); // Message ref
            payload.push_back("phoenix"); // Channel
            payload.push_back("heartbeat"); // Event Type
            payload.push_back(std::map<std::string, std::string>{
            }); // param

            // Send
            Client_WS.send(Connection_Handle, payload.dump(), websocketpp::frame::opcode::text);
        }
    }
#endif
}
