#pragma once

class APlayerState : public AActor
{

};

class Test : public UObject
{
public:
    DEFINE_PROPERTY(UObject*, HeroDefinition);
};

class AFortPlayerState : public APlayerState
{
public:
    DEFINE_PROPERTY(UObject*, HeroType);
};

class AFortPlayerStateAthena : public AFortPlayerState
{
public:
    DEFINE_PROPERTY(uint8_t, TeamIndex);
};
