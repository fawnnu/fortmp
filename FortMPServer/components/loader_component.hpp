#pragma once

class loader_component
{
public:
    loader_component();

    static void start_match(UFortPlaylistAthena* in_playlist = nullptr);

    static bool initialize_playlist(UFortPlaylistAthena* in_playlist);
    static void update_max_players(int MaxPlayers);
    static void spawn_floor_loot(UObject* Class, FString LootTier);
    static void initialize_vending_machines();
    static void initialize_llamas();

    inline static bool is_server_ready = false;

private:
    inline static nlohmann::json req_body = R"({
            "content": null,
            "embeds": [
             {
                 "title": "Server Status",
                  "description": "Servers are up on 4.2, press play or open erafn.org to join :white_check_mark:",
                  "color": null
               }
            ],
            "attachments": []
        })"_json;
};

namespace loader_component_hooks
{
    // Called when a streaming level is loaded
    DEFINE_FUNCTION_HOOK(void, UpdateLevelStreaming, (UWorld* World));
}