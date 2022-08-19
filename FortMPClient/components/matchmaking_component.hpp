#pragma once

class matchmaking_component
{
public:
    matchmaking_component();

    void on_connected(websocketpp::connection_hdl handle);

    void send_connecting(websocketpp::connection_hdl handle);
    void send_waiting(websocketpp::connection_hdl handle);
    void send_queued(websocketpp::connection_hdl handle);
    void send_assignsession(websocketpp::connection_hdl handle);
    void send_join(websocketpp::connection_hdl handle);

private:
    websocketpp::server<websocketpp::config::asio> server;

    std::string ticket_id;
    std::string session_id;
    std::string match_id;
};
