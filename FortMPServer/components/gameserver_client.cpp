#include "include.hpp"
#include "gameserver_client.hpp"

gameserver_client::gameserver_client()
{
#ifdef _RELEASE
    // Initialize status
    GameServer_Status = gameserver_status();
    
    // initialize the client
    Client_WS.init_asio();

    // Set handlers
    Client_WS.set_open_handler(websocketpp::lib::bind(&gameserver_client::OnConnected, this, websocketpp::lib::placeholders::_1));
    
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
#endif
}

void gameserver_client::OnConnected(websocketpp::connection_hdl handle)
{
#ifdef _RELEASE
    Channel = "server:63779a6f-7e6e-4c3e-bfdd-ebc136cb5f31";

    // Get the IP using ipify's API
    httplib::Client client("https://api64.ipify.org");
    auto Result = client.Get("/?format=json");
    nlohmann::json JsonResult = nlohmann::json::parse(Result->body);
    
    SendJoin(JsonResult["ip"]);
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
