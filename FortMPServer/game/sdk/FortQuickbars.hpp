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

    void ServerAddItemInternal(UFortItemDefinition* in_item_definition, FGuid& InGuid)
    {
        static UFunction* server_add_item_internal_func_function = FindObject<UFunction>(L"/Script/FortniteGame.FortQuickBars.ServerAddItemInternal");

        if (server_add_item_internal_func_function != nullptr)
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
    }
};