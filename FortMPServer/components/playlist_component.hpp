#pragma once

// We'll use this struct to properly handle legacy playlists
struct era_playlist
{
    era_playlist(std::vector<std::string> inNames, int inTeam_size, bool inbSquadPlay, int inPlaylistId)
    {
        for (std::string name : inNames)
        {
            names.push_back(name);
        }
        
        team_size = inTeam_size;
        bSquadPlay = inbSquadPlay;
        PlaylistId = inPlaylistId;
    }
    
    std::vector<std::string> names;
    int team_size;
    bool bSquadPlay;
    int PlaylistId;
};

class playlist_component
{
public:
    playlist_component();

    static void add_playlist(era_playlist playlist);
    static era_playlist* find_playlist(std::string playlist_name);
    static bool validate_playlist_for_private(UFortPlaylist* InPlaylist);
    
private:
    inline static std::vector<era_playlist> playlists;
    inline static std::vector<std::wstring> WhitelistedPlaylists;
};
