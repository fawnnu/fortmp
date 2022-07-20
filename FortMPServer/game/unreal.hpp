#pragma once

#include "unreal/unreal_math.hpp"
#include "unreal/unreal_containers.hpp"
#include "unreal/unreal_string.hpp"
#include "unreal/unreal_net.hpp"

class unreal_manager
{
public:
    static void setup_unreal_engine();
};

class UWorld;
class UObject;

inline UWorld** GWorld;
inline UObject** GEngine;

// Find UProperty/FProperty by FName
inline void* (*FindPropertyByName)(UObject* InClass, FName PropertyToFind);

// Find UFunction by FName
inline UObject* (*FindFunctionByName)(UObject* InObject, FName FunctionToFind, int);

// UObject::ProcessEvent
inline void* (*ProcessEvent)(UObject* Object, UObject* Function, void* Params);

// Returns current Engine version
inline struct FString (*GetEngineVersion)();

// Find an object by name
inline UObject* (*StaticFindObject)(UObject* Class, void* Outer, const wchar_t* Name, bool ExactClass);

// Load an object by name
inline UObject* (*StaticLoadObject)(UObject*, UObject*, const wchar_t*, const wchar_t*, int, void*, bool);

// Get parameters from StackFrame.Code
inline void (*Step)(struct FFrame*, UObject* Context, void* OutParam);

// Dereference a weak pointer
inline UObject* (*Get)(struct FWeakObjectPtr*);

// Initialize an FWeakObjectPtr with an Object
inline void (*SetFWeakObjectPtr)(struct FWeakObjectPtr*, UObject*);

// SRand
inline float (*SRand)();

namespace CoreOffsets
{
    inline DWORD Children;
    inline DWORD Next;
    inline DWORD Offset_Internal;
    inline DWORD ElementSize;
    inline DWORD PropertyFlags;
    inline DWORD StructSize;
}

namespace NetworkOffsets
{
    inline DWORD ReplicationFrame;
    inline DWORD NetworkObjects;
    inline DWORD DestroyedStartupOrDormantActors;
    inline DWORD DestroyedStartupOrDormantActors_NetDriver;
    inline DWORD TimeSeconds;
    inline DWORD IsNetRelevantForVTableIdx;
}

class UObject
{
public:
    // Pointer to the virtual function table
    void** VTable;

    // Flags describing an object instance
    int ObjectFlags;

    // Index of the object instance
    int InternalIndex;

    // Class that the object belongs to
    UObject* Class;

    // Private object name
    FName NamePrivate;

    // Outer object
    UObject* Outer;

    std::wstring GetName()
    {
        return NamePrivate.ToString();
    }

    bool HasProperty(FName in_property) const
    {
        return FindPropertyByName(Class, in_property);
    }

    bool HasFunction(FName in_function) const
    {
        return FindFunctionByName(Class, in_function, 1);
    }

    bool IsA(UObject* in_class)
    {
        // TODO: Implement inheritance
        return in_class == Class;
    }
    
    template<typename T = char, typename ...Params>
    T Call(const wchar_t* function_name, Params... args)
    {
        char offset_vector_iterator = 0;
        
        if (UObject* function_object = FindFunctionByName(this->Class, function_name, 1) ?
            FindFunctionByName(this->Class, function_name, 1) :
            StaticFindObject(nullptr, nullptr, function_name, false))
        {
            int16_t return_offset = 0;
            int16_t parms_size = 0;
            
            std::vector<int> offset_vector;
            
            for (UObject* Property = *reinterpret_cast<UObject**>(__int64(function_object) + CoreOffsets::Children); Property; Property = *reinterpret_cast<UObject**>(__int64(Property) + CoreOffsets::Next))
            {
                int offset = *reinterpret_cast<int*>(__int64(Property) + CoreOffsets::Offset_Internal);
            
                parms_size = offset + // Offset_Internal
                    *reinterpret_cast<int*>(__int64(Property) + CoreOffsets::ElementSize); // ElementSize

                offset_vector.push_back(offset);
            
                if (*reinterpret_cast<uint64_t*>(__int64(Property) + CoreOffsets::PropertyFlags) & 0x0000000000000400) //CPF_ReturnParm
                {
                    return_offset = offset;
                }
            }

            char* params = static_cast<char*>(alloca(parms_size));

            std::apply([&](auto ... argument)
            {
                ((
                    memcpy(params + offset_vector[offset_vector_iterator++], argument, sizeof(*argument))
                ), ...);
            }, std::make_tuple(&args...));
        
            ProcessEvent(this, function_object, params);

            return *reinterpret_cast<T*>(__int64(params) + return_offset);
        }
        
        // Warn that the function does not exist
        spdlog::error(L"Unable to call function '{}' due to no function existing",
            function_name);

        return {};
    }
};

class UEnum : public UObject
{
public:
    uint8_t GetEnumIndex(std::wstring EnumName)
    {
        // Get list of all enums
        TArray<TPair<FName, __int64>> NamesList = *reinterpret_cast<TArray<TPair<FName, __int64>>*>(__int64(this) + 0x40);

        for (int i = 0; i < NamesList.Num(); i++)
        {
            // Enum name
            std::wstring EnumKey = NamesList[i].Key.ToString();
            
            if (EnumKey.erase(EnumKey.find(this->GetName()), this->GetName().length() + 2) == EnumName)
            {
                // If given enum name matches the current enum name (with E[...]:: removed), return current index
                return i;
            }
        }

        return 0;
    }
};

struct FFrame
{
    unsigned char Unk00[0x10];

    UObject* Node;
    UObject* Object;
    
    // Code
    uint8_t* Code;
    
    // Pointer to parameters in frame
    uint8_t* Locals;

    void Step(UObject* Context, void* OutParam)
    {
        ::Step(this, Context, OutParam);
    }
};

template<typename T = UObject>
T* FindObject(const wchar_t* ObjectName, bool bLoadObject = false, bool bUseAnyPackage = false)
{
    UObject* Object = StaticFindObject(nullptr, bUseAnyPackage ? reinterpret_cast<void*>(-1) : nullptr, ObjectName, false);

    if (!Object && bLoadObject)
    {
        Object = StaticLoadObject(nullptr, nullptr, ObjectName, nullptr, 0, nullptr, true);
    }

    if (Object == nullptr)
    {
        spdlog::warn(L"Object '{}' not found",
            ObjectName);
    }

    return reinterpret_cast<T*>(Object);
}

class UFunction : public UObject
{
public:
    void* get_native_function()
    {
        // Offset of native function in a UFunction struct
        static int function_offset = *reinterpret_cast<int*>(__int64(this->Class) + CoreOffsets::StructSize) - sizeof(void*);

        return *reinterpret_cast<void**>(__int64(this) + function_offset); 
    }
};

struct FWeakObjectPtr
{
    FWeakObjectPtr() = default;

    FWeakObjectPtr(UObject* in_object)
    {
        SetFWeakObjectPtr(this, in_object);
    }
    
    UObject* Get()
    {
        return ::Get(this);
    }
    
    int	ObjectIndex;
    int ObjectSerialNumber;
};

template <typename T = UObject>
struct TSoftObjectPtr
{
    T* Get()
    {
        T* Object = static_cast<T*>(WeakPtr.Get());

        if (!Object && ObjectID.AssetPathName.ComparisonIndex != 0)
        {
            Object = FindObject<T>(ObjectID.AssetPathName.ToString().c_str(), true);
        }

        return Object;
    }

    FORCEINLINE T* operator->()
    {
        return Get();
    }
    
    FORCEINLINE T* operator->() const
    {
        return Get();
    }
    
    mutable FWeakObjectPtr WeakPtr;
    
    mutable int TagAtLastTest;

    FSoftObjectPath ObjectID;
};

template<class T>
struct TWeakObjectPtr : FWeakObjectPtr
{
    TWeakObjectPtr() = default;

    TWeakObjectPtr(T* in_object)
    {
        SetFWeakObjectPtr(this, in_object);
    }
    
    T* Get()
    {
        return static_cast<T*>(FWeakObjectPtr::Get());
    }

    bool operator==(TWeakObjectPtr<T> cmp)
    {
        return ObjectIndex == cmp.ObjectIndex && ObjectSerialNumber == cmp.ObjectSerialNumber;
    }
};

template <typename T>
struct TSharedPtr
{
    T* Object;

    int32_t SharedReferenceCount;
    int32_t WeakReferenceCount;

    T* Get()
    { 
        return Object;
    }
};

struct FActorSpawnParameters
{
    char Unk00[0x40];
};

// Spawn an actor
inline UObject* (*SpawnActor)(UWorld* World, UObject* Class, FVector* Location, FRotator* Rotation, const FActorSpawnParameters& Params);

template<typename T = UObject>
T* SpawnDefaultActor(UWorld* World, UObject* Class)
{
    FActorSpawnParameters spawn_parameters {};

    FVector spawn_location = FVector { 0, 0, 0 };
    FRotator spawn_rotation = FRotator { 0, 0, 0 };

    return reinterpret_cast<T*>(SpawnActor(World, Class, &spawn_location, &spawn_rotation, spawn_parameters));
}

inline float GetEngineVersionEasy()
{
    const std::wstring engine_version = GetEngineVersion().c_str();
    
    return std::stod(engine_version.substr(0, 4)) * 100;
}

inline float GetFortniteVersionEasy()
{
    const std::wstring engine_version = GetEngineVersion().c_str();
        
    return std::stod(engine_version.substr(34, 4));
}