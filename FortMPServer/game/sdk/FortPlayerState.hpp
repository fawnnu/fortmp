#pragma once

class APlayerState : public UObject
{

};

class Test : public UObject
{
public:
    DEFINE_PROPERTY(UObject*, HeroDefinition);
};

class AFortPlayerState : public UObject
{
public:
    DEFINE_PROPERTY(UObject*, HeroType);
};

class AFortPlayerStateAthena : public AFortPlayerState
{
public:
    DEFINE_PROPERTY(uint8_t, TeamIndex);
};
