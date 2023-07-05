#include "include.hpp"
#include "phoenix_component.hpp"

phoenix_message::phoenix_message(int InMessageReference, phoenix_channel* InOwnerChannel)
{
    MessageReference = InMessageReference;
    OwnerChannel = InOwnerChannel;
}

bool phoenix_message::IsValid()
{
    return MessageReference > -1 && OwnerChannel;
}

void phoenix_message::Receive(OnReceiveFunc Callback)
{
    if (IsValid())
    {
        if (phoenix_connection* OwnerConnection = OwnerChannel->OwnerConnection)
        {
            if (!OwnerConnection->MessagesMap.contains(MessageReference))
            {
                OwnerConnection->MessagesMap[MessageReference] = Callback;
            }
        }
    }
}

phoenix_channel::phoenix_channel(std::string InChannel, phoenix_connection* InOwnerConnection)
{
    Channel = InChannel;
    OwnerConnection = InOwnerConnection;
}

phoenix_message phoenix_channel::Push(std::string EventName, nlohmann::json Payload)
{
    if (OwnerConnection)
    {
        websocketpp::lib::error_code ec;
        if (connection_obj&& Connection = OwnerConnection->Client_WS.get_con_from_hdl(OwnerConnection->Connection_Handle, ec);
            Connection && Connection->get_state() == connection_state::open)
        {
            nlohmann::json payload = R"([])"_json;

            // Copy the message before bumping it
            const int CurrentMessageReference = OwnerConnection->MessageReferenceCounter;

            payload.push_back(nullptr); // Channel Ref
            payload.push_back(OwnerConnection->MessageReferenceCounter++); // Message ref
            payload.push_back(Channel); // Channel
            payload.push_back(EventName); // Event Type
            payload.push_back(Payload); // param

            // Send
            OwnerConnection->Client_WS.send(OwnerConnection->Connection_Handle, payload.dump(), websocketpp::frame::opcode::text);
            
            return {CurrentMessageReference, this};
        }
    }
    
    return {-1, nullptr};
}

phoenix_connection::phoenix_connection()
{
    
}

phoenix_connection::phoenix_connection(std::string InNameID)
{
    // Initialize message counter
    MessageReferenceCounter = 0;
    
    // Set Name ID
    NameID = InNameID;   

    // Add self to the list of existing phoenix connections
    Connections.push_back(this);

    // initialize the client
    Client_WS.init_asio();

    // Set Message handler
    Client_WS.set_message_handler([this](websocketpp::connection_hdl, message_obj Message)
    {
        // Make sure the message is valid
        if (Message)
        {
            std::string MessagePayload = Message->get_payload();

            try
            {
                nlohmann::json ParsedPayload = nlohmann::json::parse(MessagePayload);

                // Make sure it's a phoenix message
                if (ParsedPayload.is_array() && ParsedPayload.size() == 5)
                {
                    int MessageReference = ParsedPayload.at(1);
                    std::string ChannelName = ParsedPayload.at(2);
                    std::string Event = ParsedPayload.at(3);
                    nlohmann::json PhoenixPayload = ParsedPayload.at(4);

                    // If we're tracking this message reference and event, process the message
                    if (MessagesMap.contains(MessageReference))
                    {
                        if (phoenix_channel* PhoenixChannel = Channel(ChannelName))
                        {
                            reinterpret_cast<OnReceiveFunc>(MessagesMap[MessageReference])(PhoenixChannel, PhoenixPayload);

                            MessagesMap.erase(MessagesMap.find(MessageReference));
                        }
                    }
                }
            }
            catch (nlohmann::json::parse_error& Error)
            {
            }
        }
    });
    
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

            // Send heartbeat;
            CastedParameters.Connection->Channel("phoenix")->Push("heartbeat", {});
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

phoenix_channel* phoenix_connection::Channel(std::string ChannelName)
{
    // Try and find this channel
    if (const auto Iterator = std::find(Channels.begin(), Channels.end(), ChannelName); Iterator != Channels.end())
    {
        return *Iterator;
    }

    // Create a new object
    phoenix_channel* NewChannel = new phoenix_channel(ChannelName, this);

    // Add it to the list of channels
    Channels.push_back(NewChannel);

    // Return NewChannel
    return NewChannel;
}