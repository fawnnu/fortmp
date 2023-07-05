#pragma once

// Some typedefs
typedef websocketpp::client<websocketpp::config::asio_tls_client> client_websocket;
typedef std::shared_ptr<websocketpp::connection<websocketpp::config::asio_tls_client>> connection_obj;
typedef websocketpp::config::asio_client::message_type::ptr message_obj;
typedef websocketpp::session::state::value connection_state;
typedef void(*OnOpenFunc)(class phoenix_connection* connection);
typedef void(*OnReceiveFunc)(class phoenix_channel* channel, nlohmann::json response);

class phoenix_message
{
public:
    // Constructors
    phoenix_message() = default;
    phoenix_message(int InMessageReference, phoenix_channel* InOwnerChannel);

    // Validity checks
    bool IsValid();

    // Get answer
    void Receive(OnReceiveFunc Callback);

    int MessageReference;
    phoenix_channel* OwnerChannel;
};

class phoenix_channel
{
public:
    // Constructors
    phoenix_channel() = default;
    phoenix_channel(std::string InChannel, phoenix_connection* InOwnerConnection);

    // Push implementation: send a message
    phoenix_message Push(std::string EventName, nlohmann::json Payload);
    
    // Name of this channel
    std::string Channel;

    // Pointer to the connection object that created this channel
    phoenix_connection* OwnerConnection;
};

// Overload operator== for phoenix_channel pointers
FORCEINLINE bool operator==(phoenix_channel* Self, std::string ChannelName)
{
    return Self->Channel == ChannelName;
}

class phoenix_connection
{
public:
    // Constructors
    phoenix_connection();
    phoenix_connection(std::string InNameID);

    // Connect
    void Connect(std::string InURL, OnOpenFunc Callback);

    // Channel
    phoenix_channel* Channel(std::string ChannelName);
    
    // Client WS object
    client_websocket Client_WS;

    // Name identifier
    std::string NameID;

    // Connection handle
    websocketpp::connection_hdl Connection_Handle;

    // Message Reference Counter
    int MessageReferenceCounter;
    
    // Map for messages and their callbacks
    std::map<int, PVOID> MessagesMap;

    // Array of channels
    std::vector<phoenix_channel*> Channels;
    
    // Static pointer to array of self
    inline static std::vector<phoenix_connection*> Connections;
};