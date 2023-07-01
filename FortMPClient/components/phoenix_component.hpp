#pragma once

class phoenix_connection
{
public:
    phoenix_connection();
    phoenix_connection(std::string InNameID);

    // Connect
    typedef void(*OnOpenFunc)(phoenix_connection* connection);
    void Connect(std::string InURL, OnOpenFunc Callback);
    
    // Client WS object
    websocketpp::client<websocketpp::config::asio_tls_client> Client_WS;

    // Name identifier
    std::string NameID;

    // Connection handle
    websocketpp::connection_hdl Connection_Handle;
    
    // Static pointer to self
    inline static std::vector<phoenix_connection*> Connections;
};