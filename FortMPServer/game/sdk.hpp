#pragma once

#define DEFINE_PROPERTY(PropertyType, PropertyName)                                                                             \
    PropertyType##& PropertyName##()                                                                                            \
    {                                                                                                                           \
        static void* native_property = FindPropertyByName(this->Class, #PropertyName);                                          \
        static int native_property_offset = *reinterpret_cast<int*>(__int64(native_property) + CoreOffsets::Offset_Internal);   \
                                                                                                                                \
        return *reinterpret_cast<PropertyType*>(__int64(this) + native_property_offset);                                        \
    }

#define DEFINE_BIT_PROPERTY(PropertyType, PropertyName)                                                                                                             \
    PropertyType PropertyName##()                                                                                                                                \
    {                                                                                                                                                               \
        static void* native_property = FindPropertyByName(this->Class, #PropertyName);                                                                              \
        static int native_property_offset = *reinterpret_cast<int*>(__int64(native_property) + CoreOffsets::Offset_Internal);                                       \
                                                                                                                                                                    \
        auto byte_value = reinterpret_cast<PropertyType*>(__int64(this) + native_property_offset) + *reinterpret_cast<uint8_t*>(__int64(native_property) + 0x71);   \
                                                                                                                                                                    \
        return !!(*byte_value & *reinterpret_cast<uint8_t*>(__int64(native_property) + 0x73));                                                                      \
    }

#define DEFINE_PROPERTY_BY_OFFSET(PropertyType, PropertyName, PropertyOffset)         \
    PropertyType##& PropertyName##()                                                  \
    {                                                                                 \
        return *reinterpret_cast<PropertyType*>(__int64(this) + PropertyOffset);      \
    }

#define DEFINE_STATIC_CLASS(ClassFullName)\
    static UObject* StaticClass()\
    {\
        static UObject* static_class = FindObject(ClassFullName);\
        \
        return static_class;\
    }

#define DEFINE_STATIC_CLASS_LOAD(ClassFullName)\
    static UObject* StaticClass()\
    {\
        static UObject* static_class = FindObject(ClassFullName);\
        \
        if (static_class == nullptr)\
        {\
            static_class = FindObject(ClassFullName, true);\
        }\
        \
        return static_class;\
    }

#define GET_PARAMETER(PropertyType, PropertyName)                                                                                   \
    static void* PropertyName##_property = FindPropertyByName(StackFrame.Node, #PropertyName);                                      \
    static int PropertyName##_offset = *reinterpret_cast<int*>(__int64(PropertyName##_property) + CoreOffsets::Offset_Internal);    \
                                                                                                                                    \
    PropertyType##& PropertyName## = *reinterpret_cast<PropertyType##*>(StackFrame.Locals + PropertyName##_offset);

#define UNPACK_BP_PARAMS()\
    std::vector<char*> all_params;\
    for (UObject* Property = *reinterpret_cast<UObject**>(__int64(StackFrame.Node) + CoreOffsets::Children); Property; Property = *reinterpret_cast<UObject**>(__int64(Property) + CoreOffsets::Next))\
    {\
        all_params.push_back(reinterpret_cast<char*>(alloca(128)));\
        \
        StackFrame.Step(StackFrame.Object, &all_params[all_params.size() - 1]);\
    }\
    int current_param = 0;\
    \
    StackFrame.Code++;\

#define READ_BP_PARAM(PropertyType, PropertyName)\
    PropertyType##& PropertyName## = *reinterpret_cast<PropertyType##*>(&all_params[current_param++]);

#define GET_PROCESS_EVENT_PARAMETER(PropertyType, PropertyName)                                                                     \
    static void* PropertyName##_property = FindPropertyByName(Function, #PropertyName);                                             \
    static int PropertyName##_offset = *reinterpret_cast<int*>(__int64(PropertyName##_property) + CoreOffsets::Offset_Internal);    \
                                                                                                                                    \
    PropertyType##& PropertyName## = *reinterpret_cast<PropertyType##*>(__int64(Params) + PropertyName##_offset);

#define GET_PARAMETER_SAFE(PropertyType, PropertyName, DefaultValue)\
    static void* PropertyName##_property = FindPropertyByName(StackFrame.Node, #PropertyName);                                      \
    PropertyType PropertyName## = DefaultValue;                                                                                  \
    if (PropertyName##_property)                                                                                                    \
    {                                                                                                                               \
        static int PropertyName##_offset = *reinterpret_cast<int*>(__int64(PropertyName##_property) + CoreOffsets::Offset_Internal);\
                                                                                                                                    \
        PropertyName## = *reinterpret_cast<PropertyType##*>(StackFrame.Locals + PropertyName##_offset);                             \
    }                                                                                                                               

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

#define OVERLOAD_NEW_OPERATOR()\
    FORCEINLINE void* operator new(size_t)\
    {\
        auto Block = malloc(GetStructSize());\
        ZeroMemory(Block, GetStructSize());\
        return Block;\
    }

#define DEFINE_STRUCT_PROPERTY(PropertyType, PropertyName)                                                                      \
    PropertyType##& PropertyName##()                                                                                            \
    {                                                                                                                           \
        static void* native_property = FindPropertyByName(GetClass(), #PropertyName);                                           \
        static int native_property_offset = *reinterpret_cast<int*>(__int64(native_property) + CoreOffsets::Offset_Internal);   \
                                                                                                                                \
        return *reinterpret_cast<PropertyType*>(__int64(this) + native_property_offset);                                        \
    }

#define STATIC_HASPROPERTY(Object, PropertyName)\
    static bool bExists_##PropertyName = Object->HasProperty(#PropertyName)

#define STATIC_FINDFUNCTION(FunctionName)\
    static FName NAME_##FunctionName##(#FunctionName);\
    \
    static UObject* Function_##FunctionName = FindFunctionByName(this->Class, NAME_##FunctionName, 1);

#define STATIC_FINDFUNCTIONOBJECT(FunctionName)\
    static UObject* FunctionObject = FindObject(FunctionName);

#include "sdk/Engine/GameFramework/Actor.hpp"
#include "sdk/Engine/Interface.hpp"
#include "sdk/FortniteGame/Items/FortItem.hpp"
#include "sdk/Engine/GameplayTags.hpp"
#include "sdk/Engine/Texture.hpp"
#include "sdk/Engine/Net/NetConnection.hpp"
#include "sdk/Engine/EngineSettings.hpp"

#include "sdk/FortniteGame/FortMutator.hpp"
#include "sdk/FortniteGame/Player/FortPlayerState.hpp"
#include "sdk/FortniteGame/Pawns/FortPlayerPawn.hpp"
#include "sdk/FortniteGame/Player/FortPlayerController.hpp"
#include "sdk/FortniteGame/Athena/FortSafezone.hpp"
#include "sdk/FortniteGame/FortGameState.hpp"
#include "sdk/FortniteGame/FortGameMode.hpp"
#include "sdk/FortniteGame/FortAI.hpp"
#include "sdk/FortniteGame/Athena/FortPlaylist.hpp"
#include "sdk/FortniteGame/Abilities/FortAbility.hpp"
#include "sdk/FortniteGame/Athena/FortZiplines.hpp"
#include "sdk/FortniteGame/Athena/FortAthenaSupplyDrop.hpp"

#include "sdk/FortniteGame/Items/FortPickup.hpp"
#include "sdk/Engine/Net/NetDriver.hpp"
#include "sdk/Engine/World.hpp"
#include "sdk/Engine/DataTable.hpp"
#include "sdk/Engine/CurveTable.hpp"
#include "sdk/FortniteGame/Athena/GameData.hpp"
#include "sdk/Engine/Kismet/KismetLibrary.hpp"
#include "sdk/FortniteGame/Athena/MapInfo.hpp"
#include "sdk/Engine/AI/AI.hpp"

#include "sdk/FortniteGame/Building/BuildingActor.hpp"
#include "sdk/Engine/Kismet/GameplayStatics.hpp"
#include "sdk/FortniteGame/FortVolume.hpp"
#include "sdk/Engine/GameplayAbilities/AbilitySystemComponent.hpp"
#include "sdk/FortniteGame/Items/FortLoot.hpp"
#include "sdk/FortniteGame/Weapons/FortWeapon.hpp"
#include "sdk/FortniteGame/Items/FortQuickbars.hpp"
#include "sdk/FortniteGame/Athena/FortAircraft.hpp"
#include "sdk/FortniteGame/Athena/FortAthenaEvents.hpp"
#include "sdk/FortniteGame/Weapons/FortDecoTool.hpp"

#include "sdk/Engine/Components/ActorComponent.hpp"
#include "sdk/FortniteGame/Athena/FortCreative.hpp"
#include "sdk/FortniteGame/Items/FortQuest.hpp"

#include "sdk/Engine/Net/Qos.hpp"