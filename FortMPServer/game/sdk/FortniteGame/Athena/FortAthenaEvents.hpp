#pragma once

class ABP_GeodeScripting_C : public AActor
{
public:
    
    void LaunchSequence()
    {
        STATIC_FINDFUNCTION(LaunchSequence)

        Call(Function_LaunchSequence);
    }
};