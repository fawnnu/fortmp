#pragma once

class APlayerState : public UObject
{

};

class AFortPlayerStateAthena : public APlayerState
{
public:
    DEFINE_PROPERTY(uint8_t, TeamIndex);

    DEFINE_PROPERTY(UObject*, HeroType);
};
