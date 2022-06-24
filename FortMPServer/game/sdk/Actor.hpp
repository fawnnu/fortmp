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

    void SetOwner(UObject* NewOwner)
    {
        static UFunction* function = FindObject<UFunction>(L"/Script/Engine.Actor.SetOwner");

        // TODO: use Call
        ProcessEvent(this, function, &NewOwner);
    }
};
