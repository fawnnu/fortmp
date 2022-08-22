#pragma once

class matchmaking
{
public:
    static inline nlohmann::json matchmaking_response = R"({
        "id": "",
        "ownerId": "",
        "ownerName": "[DS]fortnite-liveeugcec1c2e30ubrcore0a-z8hj-1968",
        "serverName": "[DS]fortnite-liveeugcec1c2e30ubrcore0a-z8hj-1968",
        "serverAddress": "",
        "serverPort": 0,
        "maxPublicPlayers": 220,
        "openPublicPlayers": 175,
        "maxPrivatePlayers": 0,
        "openPrivatePlayers": 0,
        "attributes": {
          "REGION_s": "EU",
          "GAMEMODE_s": "FORTATHENA",
          "ALLOWBROADCASTING_b": true,
          "SUBREGION_s": "GB",
          "DCID_s": "FORTNITE-LIVEEUGCEC1C2E30UBRCORE0A-14840880",
          "tenant_s": "Fortnite",
          "MATCHMAKINGPOOL_s": "Any",
          "STORMSHIELDDEFENSETYPE_i": 0,
          "HOTFIXVERSION_i": 0,
          "PLAYLISTNAME_s": "Playlist_DefaultSolo",
          "SESSIONKEY_s": "",
          "TENANT_s": "Fortnite",
          "BEACONPORT_i": 15009
        },
        "publicPlayers": [],
        "privatePlayers": [],
        "totalPlayers": 45,
        "allowJoinInProgress": false,
        "shouldAdvertise": false,
        "isDedicated": true,
        "usesStats": false,
        "allowInvites": false,
        "usesPresence": false,
        "allowJoinViaPresence": true,
        "allowJoinViaPresenceFriendsOnly": false,
        "buildUniqueId": 0,
        "lastUpdated": "2022-02-28T06:38:50.565Z",
        "started": false
      })"_json;
    
    static inline std::string gameserver_addr;
    static inline int gameserver_port;
    static inline std::string current_build_id;
    
    matchmaking() = default;

    matchmaking(httplib::Server& in_app)
    {
        in_app.Get(R"(/fortnite/api/game/v2/matchmakingservice/ticket/player/(.*))", [](const httplib::Request& req, httplib::Response& res)
        {
            current_build_id = req.get_param_value("bucketId").substr(0, req.get_param_value("bucketId").find(":"));
            
            const nlohmann::json ticket = {
                {"serviceUrl", "ws://127.0.0.1:5556"},
                {"ticketType", "mms-player"},
                {"payload", "69="},
                {"signature", "420="}
            };

            res.set_content(ticket.dump(), "application/json");
        });

        in_app.Get(R"(/fortnite/api/game/v2/matchmaking/account/(.*)/session/(.*))", [](const httplib::Request& req, httplib::Response& res)
        {
            const nlohmann::json response = {
                {"accountId", req.matches[1]},
                {"sessionId", req.matches[2]},
                {"key", "AOJEv8uTFmUh7XM2328kq9rlAzeQ5xzWzPIiyKn2s7s="}
            };

            res.set_content(response.dump(), "application/json");
        });
        
        in_app.Get(R"(/fortnite/api/matchmaking/session/(.*))", [](const httplib::Request& req, httplib::Response& res)
        {
            // we'll use the current timestamp to generate ownerId and session key
            const int64_t current_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            
            matchmaking_response["serverAddress"] = api_utils::get_ip_from_hostname(gameserver_addr);
            matchmaking_response["serverPort"] = gameserver_port;

            matchmaking_response["id"] = req.matches[1];
            matchmaking_response["buildUniqueId"] = current_build_id;

            matchmaking_response["ownerId"] = std::to_string(std::hash<std::string>{}(std::to_string(current_timestamp)));
            matchmaking_response["attributes"]["SESSIONKEY_s"] = std::to_string(std::hash<std::string>{}(std::to_string(current_timestamp + 1)));
                
            res.set_content(matchmaking_response.dump(), "application/json");
        });

        in_app.Post(R"(/fortnite/api/matchmaking/session/(.*)/join)", [](const httplib::Request& req, httplib::Response& res)
        {
             res.status = 204;
        });

        in_app.Post("/fortnite/api/matchmaking/session/matchMakingRequest", [](const httplib::Request& req, httplib::Response& res)
        {
            res.set_content("[]", "application/json");
        });
    }
};