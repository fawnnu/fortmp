#pragma once

class player_tab
{
public:
    static void render();
    
    static void render_player(AFortPlayerController* player_controller);

    // Selected player
    inline static int selected_player_idx = 0;
};
