#pragma once

class moderation_component
{
public:
    moderation_component();
};

namespace moderation_component_hooks
{
    DEFINE_FUNCTION_HOOK(AFortPlayerController*, Login, (AFortGameModeAthena* Self, UPlayer* NewPlayer, int InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueID, FString& ErrorMessage));
}
