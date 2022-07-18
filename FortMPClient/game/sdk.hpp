#pragma once

#define DEFINE_PROPERTY(PropertyType, PropertyName)                                                                             \
    PropertyType##& PropertyName##()                                                                                            \
    {                                                                                                                           \
        static void* native_property = FindPropertyByName(this->Class, XOR(#PropertyName));                                          \
        static int native_property_offset = *reinterpret_cast<int*>(__int64(native_property) + CoreOffsets::Offset_Internal);   \
                                                                                                                                \
        return *reinterpret_cast<PropertyType*>(__int64(this) + native_property_offset);                                        \
    }

#define DEFINE_STRUCT_CLASS(ClassName)                                                                 \
    static UObject* GetClass()                                                                         \
    {                                                                                                  \
        static UObject* class_ptr = FindObject(XOR(ClassName));                                             \
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
        static void* native_property = FindPropertyByName(GetClass(), XOR(#PropertyName));                                           \
        static int native_property_offset = *reinterpret_cast<int*>(__int64(native_property) + CoreOffsets::Offset_Internal);   \
                                                                                                                                \
        return *reinterpret_cast<PropertyType*>(__int64(this) + native_property_offset);                                        \
    }

#include "sdk/Actor.hpp"
#include "sdk/World.hpp"
#include "sdk/KismetLibrary.hpp"
#include "sdk/GameplayStatics.hpp"
#include "sdk/Engine.hpp"