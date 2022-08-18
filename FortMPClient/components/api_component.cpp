#include "include.hpp"
#include "api_component.hpp"

#include "../API/lightswitch.hpp"
#include "../API/auth.hpp"
#include "../API/cloudstorage.hpp"
#include "../API/fortnitegame.hpp"
#include "../API/profile.hpp"
#include "../API/storefront.hpp"

api_component::api_component()
{
    auth::local_username = "FortMP User";
    
    new auth(server_app);
    new fortnitegame(server_app);
    new cloudstorage(server_app);
    new profile(server_app);
    new lightswitch(server_app);
    new storefront(server_app);
    
    server_app.listen("127.0.0.1", 5555);
}
