#include "include.hpp"
#include "gameserver_client.hpp"

#include "config_manager.hpp"
#include "player_component.hpp"

void HeartbeatThread()
{
    while (true)
    {
        auto ConnectionState = gameserver_client::Client->Client_WS.get_con_from_hdl(gameserver_client::Client->Connection_Handle)->get_state();

        // If the connection is open, send heartbeat
        if (ConnectionState == websocketpp::session::state::open)
        {
            gameserver_client::Client->SendHeartbeat();
        }
        
        Sleep(10000);
    }
}

gameserver_client::gameserver_client()
{
#ifdef _RELEASE
    // Initialize self
    Client = this;
    
    // Initialize status
    GameServer_Status = gameserver_status();
    
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
    auto Connection = Client_WS.get_connection("wss://ws.erafn.org/internal/game/websocket?token=bn8HUsAfhogEq6E57ledvyGReK9n8E7XP53AbRomzvItAexNB09aXiTgFhoPvUD1&vsn=2.0.0", error_code);
    Connection_Handle = Connection->get_handle();
    
    if (error_code)
    {
        spdlog::error("Failed to initialize GameServer Client with error code: {}. Era Services will not see this server.", error_code.message().c_str());
        return;
    }
    
    Client_WS.connect(Connection);

    // Start the heartbeat thread
    CreateThread(0,0, reinterpret_cast<LPTHREAD_START_ROUTINE>(&HeartbeatThread), 0,0,0);
    
    Client_WS.run();
#endif
}

void gameserver_client::OnConnected(websocketpp::connection_hdl handle)
{
#ifdef _RELEASE
    nlohmann::json server_config = config_manager::get_server_config();

    Channel = "server:" + std::string(server_config["uuid"]);

    // Get the IP using ipify's API
    // TODO: Fix vps crashes on code below
    httplib::Client client("https://api64.ipify.org");
    auto Result = client.Get("/?format=json");
    nlohmann::json JsonResult = nlohmann::json::parse(Result->body);

    GameServer_Status.ip = JsonResult["ip"];

    SendJoin(GameServer_Status.ip);
    SendUpdate();
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
        // If it's a "player" message, add the encryptiontoken to the encryptiontoken list
        if (VectorPayloadString[1] == "player")
        {
            std::string sEncryptionToken = std::string(JsonPayloadData["account_id"]) + ":" + std::string(JsonPayloadData["session_id"]);
            std::wstring wEncryptionToken = std::wstring(sEncryptionToken.begin(), sEncryptionToken.end());
           
            // Add to the valid EncryptionToken list
            player_component::EncryptionTokenList.push_back(wEncryptionToken.c_str());
        }
    }
#endif
}

void gameserver_client::SendJoin(std::string IP)
{
#ifdef _RELEASE
    nlohmann::json payload = R"([])"_json;

    payload.push_back(0); // Channel Ref
    payload.push_back(MessageRef++); // Message ref
    payload.push_back(Channel); // Channel
    payload.push_back("phx_join"); // Event Type
    payload.push_back(std::map<std::string, std::string>{
        {"ip", IP}
    }); // param

    // Send
    Client_WS.send(Connection_Handle, payload.dump(), websocketpp::frame::opcode::text);
#endif
}

void gameserver_client::SendUpdate()
{
#ifdef _RELEASE
    nlohmann::json payload = R"([])"_json;

    payload.push_back(0); // Channel Ref
    payload.push_back(MessageRef++); // Message ref
    payload.push_back(Channel); // Channel
    payload.push_back("update"); // Event Type
    payload.push_back(std::map<std::string, std::string>{
        {"accepting_players", GameServer_Status.accepting_players ? "true" : "false"},
        {"ip", GameServer_Status.ip},
        {"max_players", std::to_string(GameServer_Status.max_players)},
        {"number_of_players", std::to_string(GameServer_Status.number_of_players)},
        {"status", GameServer_Status.status}
    }); // param

    // Send
    Client_WS.send(Connection_Handle, payload.dump(), websocketpp::frame::opcode::text);
#endif
}

void gameserver_client::SendHeartbeat()
{
#ifdef _RELEASE
    nlohmann::json payload = R"([])"_json;

    payload.push_back(nullptr); // Channel Ref
    payload.push_back(MessageRef++); // Message ref
    payload.push_back("phoenix"); // Channel
    payload.push_back("heartbeat"); // Event Type
    payload.push_back(std::map<std::string, std::string>{
    }); // param

    // Send
    Client_WS.send(Connection_Handle, payload.dump(), websocketpp::frame::opcode::text);
#endif
}
