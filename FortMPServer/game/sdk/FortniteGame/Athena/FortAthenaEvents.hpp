#pragma once

class ABP_GeodeScripting_C : public AActor
{
public:

    DEFINE_STATIC_CLASS(L"/Game/Athena/Maps/Test/Events/BP_GeodeScripting.BP_GeodeScripting_C")
    
    void LaunchSequence()
    {
        STATIC_FINDFUNCTION(LaunchSequence)

        Call(Function_LaunchSequence);
    }

    void Event_TurnOffGodRay()
    {
        STATIC_FINDFUNCTION(Event_TurnOffGodRay)

        Call(Function_Event_TurnOffGodRay);
    }
    
    void Event_TurnOnGodRay()
    {
        STATIC_FINDFUNCTION(Event_TurnOnGodRay)

        Call(Function_Event_TurnOnGodRay);
    }
};