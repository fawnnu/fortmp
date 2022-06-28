#pragma once

struct FQuickBarSlotData
{
    DEFINE_STRUCT_PROPERTY(TArray<uint8_t>, AcceptedItemTypes);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.QuickBarSlotData");
};

struct FQuickBarData
{
    DEFINE_STRUCT_PROPERTY(TArray<struct FQuickBarSlotData>, QuickbarSlots);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.QuickBarData");
};

struct FQuickBar
{
    DEFINE_STRUCT_PROPERTY(FQuickBarData, DataDefinition);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.QuickBar");
};

class AFortQuickBars : public AActor
{
public:
    DEFINE_PROPERTY(FQuickBar, PrimaryQuickBar);
    DEFINE_PROPERTY(FQuickBar, SecondaryQuickBar);
    DEFINE_PROPERTY(FQuickBar, CreativeQuickBar);
};