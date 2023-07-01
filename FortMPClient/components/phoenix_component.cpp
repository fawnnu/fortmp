#include "include.hpp"
#include "phoenix_component.hpp"

phoenix_connection::phoenix_connection()
{
    
}

phoenix_connection::phoenix_connection(std::string InNameID)
{
    // Set Name ID
    NameID = InNameID;   

    // Add self to the list of existing phoenix connections
    Connections.push_back(this);

    // initialize the client
    Client_WS.init_asio();

    // Set Message handler
    //Client_WS.set_message_handler(websocketpp::lib::bind(&gameserver_client::OnMessageReceived, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
    
    Client_WS.set_tls_init_handler([this](websocketpp::connection_hdl)
    {
        return websocketpp::lib::make_shared<asio::ssl::context>(asio::ssl::context::tls_client);
    });
}

void phoenix_connection::Connect(std::string InURL, OnOpenFunc Callback)
{
    // Set OnOpen handler
    Client_WS.set_open_handler(websocketpp::lib::bind(Callback, this));
    
    websocketpp::lib::error_code error_code;
    const auto Connection = Client_WS.get_connection(InURL, error_code);
    
    if (error_code)
    {
        return;
    }

    Connection_Handle = Connection->get_handle();

    // Establish connection
    Client_WS.connect(Connection);

    // Start the heartbeat thread
    struct ThreadParameters
    {
        ThreadParameters(phoenix_connection* InConnection) : Connection(InConnection) {}
        phoenix_connection* Connection;
    };
    CreateThread(nullptr,0, [](LPVOID Parameters) -> DWORD
    {
        ThreadParameters& CastedParameters = *static_cast<ThreadParameters*>(Parameters);
        
        while (true)
        {
            // Sleep for 10 seconds 
            Sleep(10000);

            // Send heartbeat
        }
    }, new ThreadParameters(this),0,nullptr);

    // Start the client WS
    CreateThread(nullptr, 0, [](LPVOID Parameters) -> DWORD
    {
        ThreadParameters& CastedParameters = *static_cast<ThreadParameters*>(Parameters);

        CastedParameters.Connection->Client_WS.run();
        
        return 0;
    }, new ThreadParameters(this), 0, nullptr);
}