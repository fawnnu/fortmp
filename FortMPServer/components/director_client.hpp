#pragma once

class director_client
{
public:
    director_client();

    static inline std::string key;
    static inline eudp_client* client;
};
