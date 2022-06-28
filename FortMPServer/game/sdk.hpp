#pragma once

#define DEFINE_PROPERTY(PropertyType, PropertyName)                                                                             \
    PropertyType##& PropertyName##()                                                                                            \
    {                                                                                                                           \
        static void* native_property = FindPropertyByName(this->Class, #PropertyName);                                          \
        static int native_property_offset = *reinterpret_cast<int*>(__int64(native_property) + CoreOffsets::Offset_Internal);   \
                                                                                                                                \
        return *reinterpret_cast<PropertyType*>(__int64(this) + native_property_offset);                                        \
    }

#define DEFINE_PROPERTY_BY_OFFSET(PropertyType, PropertyName, PropertyOffset)         \
    PropertyType##& PropertyName##()                                                  \
    {                                                                                 \
        return *reinterpret_cast<PropertyType*>(__int64(this) + PropertyOffset);      \
    }

#define GET_PROPERTY(PropertyType, PropertyName)                                                                                    \
    static void* PropertyName##_property = FindPropertyByName(StackFrame.Node, #PropertyName);                                      \
    static int PropertyName##_offset = *reinterpret_cast<int*>(__int64(PropertyName##_property) + CoreOffsets::Offset_Internal);    \
                                                                                                                                    \
    PropertyType##& PropertyName## = *reinterpret_cast<PropertyType##*>(StackFrame.Locals + PropertyName##_offset);


#define DEFINE_STRUCT_CLASS(ClassName)                                                                 \
    static UObject* GetClass()                                                                         \
    {                                                                                                  \
        static UObject* class_ptr = FindObject(ClassName);                                             \
                                                                                                       \
        return class_ptr;                                                                              \
    }                                                                                                  \
                                                                                                       \
    static int GetStructSize()                                                                         \
    {                                                                                                  \
        return *reinterpret_cast<int*>(__int64(GetClass()) + CoreOffsets::StructSize);                 \
    }

#define DEFINE_STRUCT_PROPERTY(PropertyType, PropertyName)                                                                      \
    PropertyType##& PropertyName##()                                                                                            \
    {                                                                                                                           \
        static void* native_property = FindPropertyByName(GetClass(), #PropertyName);                                           \
        static int native_property_offset = *reinterpret_cast<int*>(__int64(native_property) + CoreOffsets::Offset_Internal);   \
                                                                                                                                \
        return *reinterpret_cast<PropertyType*>(__int64(this) + native_property_offset);                                        \
    }

#include "sdk/Actor.hpp"
#include "sdk/FortItem.hpp"

#include "sdk/FortPlayerState.hpp"
#include "sdk/FortPlayerPawn.hpp"
#include "sdk/FortPlayerController.hpp"
#include "sdk/FortGameState.hpp"
#include "sdk/FortGameMode.hpp"
#include "sdk/FortPlaylist.hpp"
#include "sdk/FortQuickbars.hpp"

#include "sdk/FortPickup.hpp"
#include "sdk/NetConnection.hpp"
#include "sdk/NetDriver.hpp"
#include "sdk/World.hpp"
#include "sdk/DataTable.hpp"
#include "sdk/KismetLibrary.hpp"

#include "sdk/BuildingActor.hpp"
#include "sdk/GameplayStatics.hpp"
#include "sdk/FortVolume.hpp"
#include "sdk/AbilitySystemComponent.hpp"
#include "sdk/FortLoot.hpp"