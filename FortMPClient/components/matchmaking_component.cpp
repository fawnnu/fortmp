#include "include.hpp"
#include "matchmaking_component.hpp"

matchmaking_component::matchmaking_component()
{
    // Initialize server
    server.init_asio();

    // add a callback for the on open event
    server.set_open_handler(websocketpp::lib::bind(&matchmaking_component::on_connected, this, websocketpp::lib::placeholders::_1));
    
    // Listen on port 5556
    server.listen(5556);

    // Start accepting connections
    server.start_accept();

    // Start
    server.run();
}

void matchmaking_component::on_connected(websocketpp::connection_hdl handle)
{
    const int64_t current_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    ticket_id = MD5{}(std::to_string(current_timestamp));
    session_id = MD5{}(std::to_string(current_timestamp + 1));
    match_id = MD5{}(std::to_string(current_timestamp + 2));
    
    send_connecting(handle);
    send_waiting(handle);
    send_queued(handle);
    send_assignsession(handle);
    send_join(handle);
}

void matchmaking_component::send_connecting(websocketpp::connection_hdl handle)
{
    std::error_code err;

    const nlohmann::json res = R"({
            "payload": {
                "state": "Connecting"
            },
            "name": "StatusUpdate"
        })"_json;

    server.send(handle, res.dump(), websocketpp::frame::opcode::text, err);
}

void matchmaking_component::send_waiting(websocketpp::connection_hdl handle)
{
    std::error_code err;

    const nlohmann::json res = R"({
            "payload": {
                "totalPlayers": 1,
                "connectedPlayers": 1,
                "state": "Waiting"
            },
            "name": "StatusUpdate"
        })"_json;

    server.send(handle, res.dump(), websocketpp::frame::opcode::text, err);
}

void matchmaking_component::send_queued(websocketpp::connection_hdl handle)
{
    std::error_code err;

    const nlohmann::json res = {
        {"payload", {
            {"ticketId", ticket_id},
            {"queuedPlayers", 0},
            {"estimatedWaitSec", 0},
            {"status", nlohmann::json({})},
            {"state", "Queued"}
        }},
        {"name", "StatusUpdate"}
    };

    server.send(handle, res.dump(), websocketpp::frame::opcode::text, err);
}

void matchmaking_component::send_assignsession(websocketpp::connection_hdl handle)
{
    std::error_code err;

    const nlohmann::json res = {
        {"payload", {
            {"matchId", match_id},
            {"state", "SessionAssignment"}
        }},
        {"name", "StatusUpdate"}
    };

    server.send(handle, res.dump(), websocketpp::frame::opcode::text, err);
}

void matchmaking_component::send_join(websocketpp::connection_hdl handle)
{
    std::error_code err;

    const nlohmann::json res = {
        {"payload", {
            {"matchId", match_id},
            {"sessionId", session_id},
            {"joinDelaySec", 1}
        }},
        {"name", "Play"}
    };

    server.send(handle, res.dump(), websocketpp::frame::opcode::text, err);
}