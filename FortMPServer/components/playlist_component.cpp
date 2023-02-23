#include "include.hpp"
#include "playlist_component.hpp"

playlist_component::playlist_component()
{
    // Legacy playlists
    if (GetEngineVersionEasy() < 420)
    {
        add_playlist(era_playlist({"", "solo", "solos", "Playlist_DefaultSolo"}, 1, false, 2));
        add_playlist(era_playlist({"duo", "duos", "Playlist_DefaultDuo", "Playlist_DefaultDuos"}, 2, true, 10));
        add_playlist(era_playlist({"squad", "squads", "Playlist_DefaultSquad", "Playlist_DefaultSquads"}, 4, true, 9));
    }
}

void playlist_component::add_playlist(era_playlist playlist)
{
    playlists.push_back(playlist);
}

era_playlist* playlist_component::find_playlist(std::string playlist_name)
{
    for (era_playlist& playlist : playlists)
    {
        for (std::string name : playlist.names)
        {
            // Convert both to lowercase and compare
            std::transform(name.begin(), name.end(), name.begin(), tolower);
            std::transform(playlist_name.begin(), playlist_name.end(), playlist_name.begin(), tolower);
            
            if (name == playlist_name)
            {
                return &playlist;
            }
        }
    }
    
    return nullptr;
}
