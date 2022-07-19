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
    DEFINE_PROPERTY(FQuickBar, PrimaryQuickBar);
    DEFINE_PROPERTY(FQuickBar, SecondaryQuickBar);
    DEFINE_PROPERTY(FQuickBar, CreativeQuickBar);

    void ServerActivateSlotInternal(uint8_t InQuickbar, int Slot)
    {
        Call(L"ServerActivateSlotInternal", InQuickbar, Slot, 0.f, false);
    }

    void ServerRemoveItemInternal(FGuid InGuid, bool bIsSwapping = false)
    {
        auto primary_quickbar_slots = TArrayManager<FQuickBarSlot>(&PrimaryQuickBar().Slots(), FQuickBarSlot::GetStructSize());

        for (int i = 0; i < primary_quickbar_slots.Num(); i++)
        {
            if (primary_quickbar_slots[i].Items().Find(InGuid))
            {
                static void* items = FindPropertyByName(FQuickBarSlot::GetClass(), L"Items");
                
                GenericArray_Remove(&primary_quickbar_slots[i].Items(), items, 0);

                for (; i < primary_quickbar_slots.Num(); i++)
                {
                    if (primary_quickbar_slots[i].Items().Num())
                    {
                        if (!bIsSwapping)
                        {
                            // Activate this slot
                            ServerActivateSlotInternal(0, i);
                        }
                        
                        return;
                    }
                }

                if (!bIsSwapping)
                {
                    // If not slot was found, activate the first slot
                    ServerActivateSlotInternal(0, 0);
                }
                       
                break;
            }
        }
    }

    void ServerAddItemInternal(UFortItemDefinition* in_item_definition, FGuid& InGuid)
    {
        auto primary_quickbar_slots_datadefinition = TArrayManager<FQuickBarSlotData>(&PrimaryQuickBar().DataDefinition().QuickbarSlots(), FQuickBarSlotData::GetStructSize());
        auto primary_quickbar_slots = TArrayManager<FQuickBarSlot>(&PrimaryQuickBar().Slots(), FQuickBarSlot::GetStructSize());

        // check if this item belongs to the primary quickbar
        for (int i = 0; i < primary_quickbar_slots_datadefinition.Num(); i++)
        {
            // our item can fit in this slot
            if (primary_quickbar_slots_datadefinition[i].AcceptedItemTypes().Find(in_item_definition->ItemType()))
            {
                // make sure the slot is free
                if (primary_quickbar_slots[i].Items().Num() == 0)
                {
                    Call(L"ServerAddItemInternal", InGuid, static_cast<uint8_t>(0), i); // good slot, add item
                    
                    break;
                }
            }
        }

        // repeat process with secondary quickbar
        auto secondary_quickbar_slots_datadefinition = TArrayManager<FQuickBarSlotData>(&SecondaryQuickBar().DataDefinition().QuickbarSlots(), FQuickBarSlotData::GetStructSize());
        auto secondary_quickbar_slots = TArrayManager<FQuickBarSlot>(&SecondaryQuickBar().Slots(), FQuickBarSlot::GetStructSize());

        for (int i = 0; i < secondary_quickbar_slots_datadefinition.Num(); i++)
        {
            if (secondary_quickbar_slots_datadefinition[i].AcceptedItemTypes().Find(in_item_definition->ItemType()))
            {
                if (secondary_quickbar_slots[i].Items().Num() == 0)
                {
                    Call(L"ServerAddItemInternal", InGuid, static_cast<uint8_t>(1), i);
                        
                    break;
                }
            }
        }
    }
};