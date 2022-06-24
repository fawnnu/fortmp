#pragma once

class patches_component
{
public:
    patches_component();
};

namespace patches_component_hook
{
    inline void (*UpdateNetStats)(UObject* NetDebugWidget);
    inline bool (*GetTagsFromPawnLocation)(AFortPoiVolume* FortPoiVolume, __int64 Tags);
    inline __int64 (*TravelPatch)(UObject* GameInstance, char a1);
    inline __int64 (*LoadingScreenPatch)(__int64 Object);

    // Prevents a crash related to updating the net debug stats widget
    void UpdateNetStats_Hook(UObject* NetDebugWidget);

    // Prevents a crash from using StartMatch
    bool GetTagsFromPawnLocation_Hook(AFortPoiVolume* FortPoiVolume, __int64 Tags);

    // Prevents a crash when travelling to a new level
    __int64 TravelPatch_Hook(UObject* GameInstance, char a1);

    // Prevents a crash related to the loading screen
    __int64 LoadingScreenPatch_Hook(__int64 Object);
};