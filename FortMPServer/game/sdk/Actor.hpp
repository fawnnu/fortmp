#pragma once

class AActor : public UObject
{
public:
    FTransform GetTransform()
    {
        return Call<FTransform>(L"GetTransform");
    }

    FVector K2_GetActorLocation()
    {
        return Call<FVector>(L"K2_GetActorLocation");
    }
    
    FRotator K2_GetActorRotation()
    {
        return Call<FRotator>(L"K2_GetActorRotation");
    }

    void SetOwner(UObject* NewOwner)
    {
        Call(L"SetOwner", NewOwner);
    }
};
