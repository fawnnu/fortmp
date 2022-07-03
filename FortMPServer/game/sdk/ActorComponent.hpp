#pragma once

class UActorComponent : public UObject
{
public:
    AActor* GetOwner()
    {
        return Call<AActor*>(L"GetOwner");
    }
};

class UFortControllerComponent_Aircraft : public UActorComponent
{
public:
    DEFINE_PROPERTY(AFortAircraft*, CurrentAircraft);
};