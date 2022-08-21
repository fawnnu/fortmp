#pragma once

class player_tab
{
public:
    static void render();
    
    static void render_player(AFortPlayerController* player_controller);

    static void give_item(AFortPlayerController* player_controller);

    // Selected player
    inline static int selected_player_idx = 0;

    // item to grant
    inline static char item_to_give[255];
};
