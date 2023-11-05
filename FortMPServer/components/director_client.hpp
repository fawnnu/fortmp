#pragma once

class director_client
{
public:
    director_client();
    static void send_ready_event();
    static void wait_for_config();
    
    static inline std::string key;
    static inline eudp_client* client;
};
