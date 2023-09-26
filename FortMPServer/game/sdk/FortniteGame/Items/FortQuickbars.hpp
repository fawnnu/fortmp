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

struct FQuickBarSlot
{
    DEFINE_STRUCT_PROPERTY(TArray<FGuid>, Items);

    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.QuickBarSlot");
};

struct FQuickBar
{
    DEFINE_STRUCT_PROPERTY(FQuickBarData, DataDefinition);
    DEFINE_STRUCT_PROPERTY(TArray<FQuickBarSlot>, Slots);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.QuickBar");
};

class AFortQuickBars : public AActor
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortQuickBars");
    
    DEFINE_PROPERTY(FQuickBar, PrimaryQuickBar);
    DEFINE_PROPERTY(FQuickBar, SecondaryQuickBar);
    DEFINE_PROPERTY(FQuickBar, CreativeQuickBar);

    void ServerActivateSlotInternal(uint8_t InQuickbar, int Slot)
    {
        STATIC_FINDFUNCTION(ServerActivateSlotInternal)
        
        Call(Function_ServerActivateSlotInternal, InQuickbar, Slot, 0.f, false);
    }

    void ServerRemoveItemInternal(FGuid InGuid, bool bIsSwapping = false)
    {
        auto primary_quickbar_slots = TArrayManager<FQuickBarSlot>(&PrimaryQuickBar().Slots(), FQuickBarSlot::GetStructSize());
        auto secondary_quickbar_slots = TArrayManager<FQuickBarSlot>(&SecondaryQuickBar().Slots(), FQuickBarSlot::GetStructSize());

        for (int i = 0; i < primary_quickbar_slots.Num(); i++)
        {
            if (primary_quickbar_slots[i].Items().Find(InGuid))
            {
                static void* items = FindPropertyByName(FQuickBarSlot::GetClass(), L"Items");
                
                GenericArray_Remove(&primary_quickbar_slots[i].Items(), items, 0);

                if (!bIsSwapping)
                {
                    // Fix dropping issue with focus
                    if (AFortPawn* player_pawn = static_cast<AFortPlayerController*>(GetOwner())->Pawn())
                    {
                        if (AFortWeapon* current_weapon = player_pawn->CurrentWeapon())
                        {
                            for (int j = 0; j < primary_quickbar_slots.Num(); j++)
                            {
                                if (primary_quickbar_slots[j].Items().Num())
                                {
                                    if (primary_quickbar_slots[j].Items()[0] == current_weapon->ItemEntryGuid())
                                    {
                                        if (j < i)
                                        {
                                            return;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    
                    for (; i < primary_quickbar_slots.Num(); i++)
                    {
                        if (primary_quickbar_slots[i].Items().Num())
                        {
                            // Activate this slot
                            ServerActivateSlotInternal(0, i);
                        
                            return;
                        }
                    }

                    // If not slot was found, activate the first slot
                    ServerActivateSlotInternal(0, 0);
                }
                       
                break;
            }
        }

        for (int i = 0; i < secondary_quickbar_slots.Num(); i++)
        {
            if (secondary_quickbar_slots[i].Items().Find(InGuid))
            {
                // remove and activate first slot
                static void* items = FindPropertyByName(FQuickBarSlot::GetClass(), L"Items");
                GenericArray_Remove(&secondary_quickbar_slots[i].Items(), items, 0);
                
                ServerActivateSlotInternal(0, 0);

                break;
            }
        }
    }
};