#pragma once

class IInterface : public UObject
{
public:
    
};

class IFortInventoryOwnerInterface : public IInterface
{
public:
    static UObject* StaticClass()
    {
        static UObject* static_class = FindObject(L"/Script/FortniteGame.FortInventoryOwnerInterface");

        return static_class;
    }
};