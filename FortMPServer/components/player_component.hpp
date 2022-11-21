#pragma once

class player_component
{
public:
    player_component();

    static AFortPlayerPawn* create_player(AFortPlayerController* player_controller, FTransform spawn_transform);

    static inline bool bEnableServerChoosePart;
};

namespace player_component_hooks
{
    // Called to set acknowledged pawn
    DEFINE_FUNCTION_HOOK(void, ServerAcknowledgePossession, (APlayerController* PlayerController, FFrame& StackFrame));
    
    // Called when a player joins the server
    DEFINE_FUNCTION_HOOK(void, PostLogin, (AFortGameModeAthena* FortGameModeAthena, APlayerController* NewPlayer));

    // Called to override the hero type
    DEFINE_FUNCTION_HOOK(UObject*, GetHeroDefinition, (UObject* Hero));

    // Called when a player attempts to emote
    DEFINE_FUNCTION_HOOK(void, ServerPlayEmoteItem, (AFortPlayerController* FortPlayerController, FFrame& StackFrame));

    // Called when a player attempts to enter a ziplines
    DEFINE_FUNCTION_HOOK(void, ServerSendZiplineState, (AFortPlayerPawn* FortPlayerPawn, FFrame& StackFrame));

    // Called to set a CharacterPart, we hook it to null it
    DEFINE_FUNCTION_HOOK(void, ServerChoosePart, (AFortPlayerPawn*, FFrame&));

    // Called when a player is disconnecting
    DEFINE_FUNCTION_HOOK(void, ServerReturnToMainMenu, (AFortPlayerController* FortPlayerController));

    // Called when a player has been spawned
    DEFINE_FUNCTION_HOOK(void, K2_OnRestartPlayer, (UObject* Object, UObject* Function, void* Params));
}
