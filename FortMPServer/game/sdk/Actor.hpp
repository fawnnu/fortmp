#pragma once

class AActor : public UObject
{
public:
    FTransform GetTransform()
    {
        static UFunction* function = FindObject<UFunction>(L"/Script/Engine.Actor.GetTransform");

        FTransform ActorTransform {};
        
        // TODO: use Call
        ProcessEvent(this, function, &ActorTransform);

        return ActorTransform;
    }

    FVector K2_GetActorLocation()
    {
        static UFunction* function = FindObject<UFunction>(L"/Script/Engine.Actor.K2_GetActorLocation");

        FVector ActorLocation {};
        
        // TODO: use Call
        ProcessEvent(this, function, &ActorLocation);

        return ActorLocation;
    }
    
    FRotator K2_GetActorRotation()
    {
        static UFunction* function = FindObject<UFunction>(L"/Script/Engine.Actor.K2_GetActorRotation");

        FRotator ActorRotation { 0, 0, 0 };
        
        // TODO: use Call
        ProcessEvent(this, function, &ActorRotation);

        return ActorRotation;
    }

    void SetOwner(UObject* NewOwner)
    {
        static UFunction* function = FindObject<UFunction>(L"/Script/Engine.Actor.SetOwner");

        // TODO: use Call
        ProcessEvent(this, function, &NewOwner);
    }
};
