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

    bool K2_SetActorRotation(FRotator NewRotation, bool bTeleportPhysics)
    {
        return Call<bool>(L"K2_SetActorRotation", NewRotation, bTeleportPhysics);
    }

    void SetOwner(UObject* NewOwner)
    {
        Call(L"SetOwner", NewOwner);
    }

    AActor* GetOwner()
    {
        return Call<AActor*>(L"GetOwner");
    }
};
