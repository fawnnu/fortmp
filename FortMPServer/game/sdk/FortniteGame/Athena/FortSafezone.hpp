#pragma once

class AFortSafeZoneIndicator : public AActor
{
public:
    DEFINE_PROPERTY(float, SafeZoneStartShrinkTime);

    DEFINE_PROPERTY(float, SafeZoneFinishShrinkTime);
};