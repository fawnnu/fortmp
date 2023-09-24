#pragma once

class AFortSafeZoneIndicator : public AActor
{
public:
    DEFINE_PROPERTY(float, SafeZoneStartShrinkTime);

    DEFINE_PROPERTY(float, SafeZoneFinishShrinkTime);

    FVector GetSafeZoneCenter()
    {
        STATIC_FINDFUNCTION(GetSafeZoneCenter)
        return Call<FVector>(Function_GetSafeZoneCenter);
    }
};