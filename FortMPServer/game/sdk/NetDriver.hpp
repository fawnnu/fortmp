#pragma once

class UNetDriver : public UObject
{
public:
    DEFINE_PROPERTY(UObject*, ReplicationDriver);
};
