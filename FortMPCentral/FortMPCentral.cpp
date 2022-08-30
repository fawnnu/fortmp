#include "include.hpp"
#include "FortMPCentral.hpp"
#include "API/matchmaking.hpp"

int main(int argc, char* argv[])
{
    spdlog::info(L"Starting central server...");
    
    new matchmaking(fortmp_central_api::server_app);

    fortmp_central_api::server_app.listen("127.0.0.1", 5557);
    
    return 0;
}
