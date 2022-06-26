#include "include.hpp"
#include "loader_component.hpp"

#include "net_component.hpp"

loader_component::loader_component()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    
    REGISTER_UFUNCTION_DETOUR(loader_component_hooks::ReadyToStartMatch, L"/Script/Engine.GameMode.ReadyToStartMatch", loader_component_hooks::ReadyToStartMatch_Hook);

    DetourTransactionCommit();
}

void loader_component::start_match(UFortPlaylistAthena* in_playlist)
{
    if (in_playlist == nullptr)
    {
        // Use default playlist if none specified
        in_playlist = FindObject<UFortPlaylistAthena>(L"/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo");
    }

    if ((*GWorld)->GameState()->HasProperty("CurrentPlaylistData"))
    {
        (*GWorld)->GameState()->CurrentPlaylistData() = in_playlist;

        (*GWorld)->GameState()->OnRep_CurrentPlaylistData();
    }
    else if ((*GWorld)->GameState()->HasProperty("CurrentPlaylistInfo"))
    {
        (*GWorld)->GameState()->CurrentPlaylistInfo().BasePlaylist() = in_playlist;

        (*GWorld)->GameState()->CurrentPlaylistInfo().OverridePlaylist() = in_playlist;

        (*GWorld)->GameState()->CurrentPlaylistInfo().PlaylistReplicationKey()++;

        (*GWorld)->GameState()->OnRep_CurrentPlaylistInfo();
    }
    else
    {
        spdlog::error(L"Couldn't set the playlist, CurrentPlaylistData and CurrentPlaylistInfo do not exist.");
        return;
    }
    
    // Make warmup longer
    (*GWorld)->AuthorityGameMode()->WarmupCountdownDuration() = 180.f;
    (*GWorld)->AuthorityGameMode()->WarmupEarlyCountdownDuration() = 180.f;

    //listen
    const FURL in_url((*GWorld)->GetName().c_str(), 7777);
                    
    if (net_component::listen(in_url))
    {
        spdlog::info(L"Match started in world '{}' and listening for connections",
            (*GWorld)->GetName().c_str());
    }
    else
    {
        spdlog::error(L"Failed to start listening in '{}' on port 7777", (*GWorld)->GetName().c_str());
    }

    // Update default pawn class
    (*GWorld)->AuthorityGameMode()->DefaultPawnClass() = FindObject(L"/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C");
}

void loader_component::load_level(FName package_name)
{
    if (std::find(loaded_levels.begin(), loaded_levels.end(), package_name) != loaded_levels.end())
    {
        return;
    }
    
    // Find the UWorld instance of the level
    UWorld* world_level = FindObject<UWorld>(package_name.ToString().c_str(), false, true);

    if (world_level)
    {
        loaded_levels.push_back(package_name);

        // All floor loot actors in the sub-level
        TArray<UObject*> floor_loot_actors = FindObject<UGameplayStatics>(L"/Script/Engine.Default__GameplayStatics")->GetAllActorsOfClass(*GWorld, FindObject(L"Tiered_Athena_FloorLoot_01_C", false, true));
        
        for (UObject* floor_loot_actor : floor_loot_actors)
        {
            // TODO: Roll floor loot
        }
    }
    else
    {
        spdlog::error(L"Failed to load level with package name '{}', no world context object exists",
            world_level->GetName().c_str());
    }
}

// Little hack to prevent ReadyToStartMatch from being run more than once
bool bReadyToStartMatch = false;

void loader_component_hooks::ReadyToStartMatch_Hook(AGameMode* GameMode, FFrame& StackFrame)
{
    if (!bReadyToStartMatch)
    {
        bReadyToStartMatch = true;
        
        auto loading_thread = [](LPVOID) -> DWORD
        {
            while (true)
            {
                static DWORD streamed_levels_size = *reinterpret_cast<DWORD*>(uintptr_t(FindObject(L"/Script/FortniteGame.FortLevelStreamingInfo")) + CoreOffsets::StructSize);
                    
                auto streamed_levels = TArrayManager((*GWorld)->GameState()->WorldManager()->StreamedLevels(), streamed_levels_size);

                if (streamed_levels.Num() > 0 && streamed_levels.Num() == loader_component::loaded_levels.size())
                {
                    // Start match
                    loader_component::start_match();
                    
                    return 0;
                }
                
                for (const auto& streamed_level : streamed_levels)
                {
                    if (streamed_level.LevelState == 6) // LSS_Ready
                    {
                        // Load the level
                        loader_component::load_level(streamed_level.PackageName);
                    }
                }
                
                Sleep(500);
            }
            
            return 0;
        };

        // Load all streaming levels on a new thread
        CreateThread(nullptr, 0, loading_thread, nullptr, 0, nullptr);
    }
    
    return ReadyToStartMatch(GameMode, StackFrame);
}
