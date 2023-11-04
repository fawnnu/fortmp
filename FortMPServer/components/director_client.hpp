#pragma once

class director_client
{
public:
    director_client();
    static void send_ready_event();

    static inline std::string key;
    static inline eudp_client* client;
};
