#pragma once

class UGameMapsSettings : public UObject
{
public:
    DEFINE_STATIC_CLASS(L"/Script/EngineSettings.GameMapsSettings")

    DEFINE_PROPERTY(FString, LocalMapOptions);
    DEFINE_PROPERTY(FSoftObjectPath, GameDefaultMap);
};