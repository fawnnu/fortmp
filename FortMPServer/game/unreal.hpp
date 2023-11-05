#pragma once

#include <codecvt>

#include "unreal/unreal_math.hpp"
#include "unreal/unreal_containers.hpp"
#include "unreal/unreal_string.hpp"
#include "unreal/unreal_net.hpp"

class unreal_manager
{
public:
    unreal_manager();
    static void pre_init();
};

class UWorld;
class UEngine;
class UObject;

inline UWorld** GWorld;
inline UEngine** GEngine;

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

// Creates an object
inline UObject* (*StaticConstructObject)(UObject* Class, UObject* Outer, FName InName, int ObjectFlags, int InternalObjectFlags, UObject* InTemplate, bool bCopyTransientsFromClassDefaults, void*, bool bAssumeTemplateIsArchetype);

// Get parameters from StackFrame.Code
inline void (*Step)(struct FFrame*, UObject* Context, void* OutParam);

// Dereference a weak pointer
inline UObject* (*Get)(struct FWeakObjectPtr*);

// Initialize an FWeakObjectPtr with an Object
inline void (*SetFWeakObjectPtr)(struct FWeakObjectPtr*, UObject*);

// Array of native functions
inline void** GNatives;

// GIsClient
inline bool* GIsClient;

// GIsServer
inline bool* GIsServer;

// SRand
inline float (*SRand)();

// TryLoadMips
inline bool (*TryLoadMips)(void* Self, int FirstMipToLoad, void** OutMipData);

// Request exit
inline void (*RequestExitWithStatus)(bool bForce, uint8_t ReturnCode);

// CreateDefaultObject
inline UObject* (*CreateDefaultObject_Internal)(UObject* SelfClass);

// Close a NetConnection
inline void (*NetConnection__Close)(class UNetConnection* Self);

namespace CoreOffsets
{
    inline DWORD Children;
    inline DWORD Next;
    inline DWORD Offset_Internal;
    inline DWORD ElementSize;
    inline DWORD PropertyFlags;
    inline DWORD StructSize;
    inline DWORD SuperStruct;
    inline DWORD LastHitch;
    inline DWORD ByteOffset;
    inline DWORD FieldMask;
}

namespace NetworkOffsets
{
    inline DWORD ReplicationFrame;
    inline DWORD NetworkObjects;
    inline DWORD DestroyedStartupOrDormantActors;
    inline DWORD DestroyedStartupOrDormantActors_NetDriver;
    inline DWORD TimeSeconds;
    inline DWORD IsNetRelevantForVTableIdx;
    inline DWORD GetNetOwnerVTableIdx;
    inline DWORD ActorReplicator;
    inline DWORD ObjectPtr;
    inline DWORD PreReplication;
    inline DWORD ComponentPreReplication;
    inline DWORD ArrayDim;
    inline DWORD RepIndex;
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

    std::string sGetName()
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        
        return converter.to_bytes(NamePrivate.ToString());
    }

    void* GetInterfaceAddress(UObject* InterfaceClass)
    {
        static uintptr_t getinterfaceaddress_address = memory_manager::find_pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 33 ? 48 8B ? 48 8B ? 48 85 ? 0F 84 ? ? ? ? 8B 82");

        return reinterpret_cast<void* (*)(UObject*, UObject*)>(getinterfaceaddress_address)(this, InterfaceClass);
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
        if (!in_class)
        {
            return false;
        }
        
        for (UObject* CurrentClass = Class; CurrentClass; CurrentClass = *reinterpret_cast<UObject**>(reinterpret_cast<uintptr_t>(CurrentClass) + CoreOffsets::SuperStruct))
        {
            if (CurrentClass == in_class)
            {
                return true;
            }
        }

        return false;
    }


    bool This_IsA(UObject* in_class)
    {
        if (!in_class)
        {
            return false;
        }
        
        for (UObject* CurrentClass = this; CurrentClass; CurrentClass = *reinterpret_cast<UObject**>(reinterpret_cast<uintptr_t>(CurrentClass) + CoreOffsets::SuperStruct))
        {
            if (CurrentClass == in_class)
            {
                return true;
            }
        }

        return false;
    }
    
    template<typename T = char, typename ...Params>
    T Call(UObject* function_object, Params... args)
    {
        char offset_vector_iterator = 0;
        
        if (function_object)
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

            if (params)
            {
                return *reinterpret_cast<T*>(__int64(params) + return_offset);
            }
            else
            {
                return T();
            }
        }

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

            if (EnumKey.find(this->GetName()) != std::wstring::npos)
            {
                if (EnumKey.erase(EnumKey.find(this->GetName()), this->GetName().length() + 2) == EnumName)
                {
                    // If given enum name matches the current enum name (with E[...]:: removed), return current index
                    return i;
                }
            }
            else
            {
                if (EnumKey == EnumName)
                {
                    return i;
                }
            }
        }

        return -1;
    }

    std::wstring GetEnumName(uint8_t EnumIndex)
    {
        // Get list of all enums
        TArray<TPair<FName, __int64>> NamesList = *reinterpret_cast<TArray<TPair<FName, __int64>>*>(__int64(this) + 0x40);
    
        std::wstring full_enum_name = NamesList[EnumIndex].Key.ToString();
        
        // Return the associated FName
        return full_enum_name.substr(full_enum_name.find_last_of(L":") + 1);
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
        int B = *Code++;

        reinterpret_cast<void (*)(UObject*, FFrame*, void*)>(GNatives[B])(Context, this, OutParam);
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

    return reinterpret_cast<T*>(Object);
}

class UProperty : public UObject
{
    
};

class UStrProperty : public UProperty
{
public:
    static UObject* StaticClass()
    {
        static UObject* static_class = FindObject(L"/Script/CoreUObject.StrProperty");

        return static_class;
    }
};

float GetEngineVersionEasy();

class UFunction : public UObject
{
public:
    void* get_native_function()
    {
        // Offset of native function in a UFunction struct
        static int function_offset = *reinterpret_cast<int*>(__int64(this->Class) + CoreOffsets::StructSize) - sizeof(void*);

        return *reinterpret_cast<void**>(__int64(this) + function_offset); 
    }

    void set_parms_size(int InParmsSize)
    {
        static UObject* UStruct__Class = FindObject(L"/Script/CoreUObject.Struct");

        static int UStruct_Size = *reinterpret_cast<int*>(uintptr_t(UStruct__Class) + CoreOffsets::StructSize);
        
        static int parms_size_offset = GetEngineVersionEasy() == 416 ? UStruct_Size + 0x8 : UStruct_Size + 0x6;

        *reinterpret_cast<int*>(uintptr_t(this) + parms_size_offset) = InParmsSize;
    }

    void set_return_value_offset(int InReturnValueOffset)
    {
        static UObject* UStruct__Class = FindObject(L"/Script/CoreUObject.Struct");

        static int UStruct_Size = *reinterpret_cast<int*>(uintptr_t(UStruct__Class) + CoreOffsets::StructSize);
        
        static int return_value_offset = GetEngineVersionEasy() == 416 ? UStruct_Size + 0xA : UStruct_Size + 0x8;

        *reinterpret_cast<int*>(uintptr_t(this) + return_value_offset) = InReturnValueOffset;
    }

    void set_properties_size(int InPropertiesSize)
    {
        *reinterpret_cast<int*>(uintptr_t(this) + CoreOffsets::Children + 8) = InPropertiesSize;
    }

    void set_function_flags(int InFunctionFlags)
    {
        static UObject* UStruct__Class = FindObject(L"/Script/CoreUObject.Struct");

        // Function flags are always the first property of UFunction
        static int function_flags_offset = *reinterpret_cast<int*>(uintptr_t(UStruct__Class) + CoreOffsets::StructSize);

        *reinterpret_cast<int*>(uintptr_t(this) + function_flags_offset) = InFunctionFlags;
    }

    void set_native_function(void* InNativeFunction)
    {
        // Offset of native function in a UFunction struct
        static int function_offset = *reinterpret_cast<int*>(__int64(this->Class) + CoreOffsets::StructSize) - sizeof(void*);

        *reinterpret_cast<void**>(__int64(this) + function_offset) = InNativeFunction; 
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
        static UObject* SoftObjectPath = FindObject(L"/Script/CoreUObject.SoftObjectPath");
        
        T* Object = static_cast<T*>(WeakPtr.Get());

        if (Object == nullptr && SoftObjectPath == nullptr)
        {
            Object = FindObject<T>(reinterpret_cast<FStringAssetReference*>(reinterpret_cast<uintptr_t>(this) + 0x10)->AssetReference.c_str(), true);
        }
        else if (Object == nullptr && ObjectID.AssetPathName.ComparisonIndex != 0 && SoftObjectPath != nullptr)
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

    bool IsValid()
    {
        return Object != nullptr;
    }
};

class FScriptInterface
{
private:
    UObject* ObjectPointer;
    void* InterfacePointer;

public:
    inline UObject* GetObject() const
    {
        return ObjectPointer;
    }

    inline UObject*& GetObjectRef()
    {
        return ObjectPointer;
    }

    inline void* GetInterface() const
    {
        return ObjectPointer != nullptr ? InterfacePointer : nullptr;
    }
};

template<class InterfaceType>
class TScriptInterface : public FScriptInterface
{
public:
    inline InterfaceType* operator->() const
    {
        return (InterfaceType*)GetInterface();
    }

    inline InterfaceType& operator*() const
    {
        return *((InterfaceType*)GetInterface());
    }

    inline operator bool() const
    {
        return GetInterface() != nullptr;
    }
};

template <typename T>
struct TScriptDelegate
{
    TScriptDelegate(T* InObject, FName InFunctionName)
    {
        Object = InObject;
        FunctionName = InFunctionName;
    }
    
    TWeakObjectPtr<T> Object;

    FName FunctionName;
};

template <typename T>
struct TScriptMulticastDelegate
{
    TArray<TScriptDelegate<T>> InvocationList;

    void Bind(const TScriptDelegate<T>& InDelegate)
    {
        bool bIsAlreadyBound = false;

        for (auto InvokedDelegate : InvocationList)
        {
            if (InvokedDelegate.FunctionName == InDelegate.FunctionName)
            {
                bIsAlreadyBound = true;
                break;
            }
        }
        
        // only bind if nothing is bound to this delegate
        if (!bIsAlreadyBound)
        {
            InvocationList.Add(InDelegate);
        }
    }
};

struct FActorSpawnParameters
{
    char Unk00[0x40];
};

enum class EPropertyClass
{
    Byte,
    Int8,
    Int16,
    Int,
    Int64,
    UInt16,
    UInt32,
    UInt64,
    UnsizedInt,
    UnsizedUInt,
    Float,
    Double,
    Bool,
    SoftClass,
    WeakObject,
    LazyObject,
    SoftObject,
    Class,
    Object,
    Interface,
    Name,
    Str,
    Array,
    Map,
    Set,
    Struct,
    Delegate,
    MulticastDelegate,
    Text,
    Enum,
};

enum EObjectFlags
{
    RF_NoFlags                         = 0x00000000,
    RF_Public                          =0x00000001,
    RF_Standalone                      =0x00000002,
    RF_MarkAsNative                    =0x00000004,
    RF_Transactional                   =0x00000008,
    RF_ClassDefaultObject              =0x00000010,
    RF_ArchetypeObject                 =0x00000020,
    RF_Transient                       =0x00000040,
    RF_MarkAsRootSet                   =0x00000080,
    RF_TagGarbageTemp                  =0x00000100,
    RF_NeedInitialization              =0x00000200,
    RF_NeedLoad                        =0x00000400,
    RF_KeepForCooker                   =0x00000800,
    RF_NeedPostLoad                    =0x00001000,
    RF_NeedPostLoadSubobjects          =0x00002000,
    RF_NewerVersionExists              =0x00004000,
    RF_BeginDestroyed                  =0x00008000,
    RF_FinishDestroyed                 =0x00010000,
    RF_BeingRegenerated                =0x00020000,
    RF_DefaultSubObject                =0x00040000,
    RF_WasLoaded                       =0x00080000,
    RF_TextExportTransient             =0x00100000,
    RF_LoadCompleted                   =0x00200000,
    RF_InheritableComponentTemplate    = 0x00400000,
    RF_DuplicateTransient              =0x00800000,
    RF_StrongRefOnFrame                =0x01000000,
    RF_NonPIEDuplicateTransient        =0x02000000,
    RF_Dynamic                         =0x04000000,
    RF_WillBeLoaded                    =0x08000000,
    RF_HasExternalPackage              =0x10000000,
};

enum EPropertyFlags : uintptr_t
{
    CPF_None                              = 0,
    CPF_Edit                              = 0x0000000000000001,
    CPF_ConstParm                         = 0x0000000000000002,
    CPF_BlueprintVisible                  = 0x0000000000000004,
    CPF_ExportObject                      = 0x0000000000000008,
    CPF_BlueprintReadOnly                 = 0x0000000000000010,
    CPF_Net                               = 0x0000000000000020,
    CPF_EditFixedSize                     = 0x0000000000000040,
    CPF_Parm                              = 0x0000000000000080,
    CPF_OutParm                           = 0x0000000000000100,
    CPF_ZeroConstructor                   = 0x0000000000000200,
    CPF_ReturnParm                        = 0x0000000000000400,
    CPF_DisableEditOnTemplate             = 0x0000000000000800,
    CPF_Transient                         = 0x0000000000002000,
    CPF_Config                            = 0x0000000000004000,
    CPF_DisableEditOnInstance             = 0x0000000000010000,
    CPF_EditConst                         = 0x0000000000020000,
    CPF_GlobalConfig                      = 0x0000000000040000,
    CPF_InstancedReference                = 0x0000000000080000,
    CPF_DuplicateTransient                = 0x0000000000200000,
    CPF_SaveGame                          = 0x0000000001000000,
    CPF_NoClear                           = 0x0000000002000000,
    CPF_ReferenceParm                     = 0x0000000008000000,
    CPF_BlueprintAssignable               = 0x0000000010000000,
    CPF_Deprecated                        = 0x0000000020000000,
    CPF_IsPlainOldData                    = 0x0000000040000000,
    CPF_RepSkip                           = 0x0000000080000000,
    CPF_RepNotify                         = 0x0000000100000000,
    CPF_Interp                            = 0x0000000200000000,
    CPF_NonTransactional                  = 0x0000000400000000,
    CPF_EditorOnly                        = 0x0000000800000000,
    CPF_NoDestructor                      = 0x0000001000000000,
    CPF_AutoWeak                          = 0x0000004000000000,
    CPF_ContainsInstancedReference        = 0x0000008000000000,
    CPF_AssetRegistrySearchable           = 0x0000010000000000,
    CPF_SimpleDisplay                     = 0x0000020000000000,
    CPF_AdvancedDisplay                   = 0x0000040000000000,
    CPF_Protected                         = 0x0000080000000000,
    CPF_BlueprintCallable                 = 0x0000100000000000,
    CPF_BlueprintAuthorityOnly            = 0x0000200000000000,
    CPF_TextExportTransient               = 0x0000400000000000,
    CPF_NonPIEDuplicateTransient          = 0x0000800000000000,
    CPF_ExposeOnSpawn                     = 0x0001000000000000,
    CPF_PersistentInstance                = 0x0002000000000000,
    CPF_UObjectWrapper                    = 0x0004000000000000,
    CPF_HasGetValueTypeHash               = 0x0008000000000000,
    CPF_NativeAccessSpecifierPublic       = 0x0010000000000000,
    CPF_NativeAccessSpecifierProtected    = 0x0020000000000000,
    CPF_NativeAccessSpecifierPrivate      = 0x0040000000000000,
    CPF_SkipSerialization                 = 0x0080000000000000,
};

enum class ENativeBool
{
    NotNative,
    Native
};

struct FBoolPropertyParams 
{
    EPropertyClass   Type;
    const char*      NameUTF8;
    int     ObjectFlags;
    uintptr_t   PropertyFlags;
    int            ArrayDim;
    const char*      RepNotifyFuncUTF8;
    int           ElementSize;
    ENativeBool      NativeBool;
    SIZE_T           SizeOfOuter;
    void           (*SetBitFunc)(void* Obj);
};

// Spawn an actor
inline class AActor* (*SpawnActor)(UWorld* World, UObject* Class, FVector* Location, FRotator* Rotation, const FActorSpawnParameters& Params);

template<typename T = UObject>
T* SpawnDefaultActor(UWorld* World, UObject* Class)
{
    FActorSpawnParameters spawn_parameters {};

    FVector spawn_location = FVector { 0, 0, 0 };
    FRotator spawn_rotation = FRotator { 0, 0, 0 };

    return reinterpret_cast<T*>(SpawnActor(World, Class, &spawn_location, &spawn_rotation, spawn_parameters));
}

inline float GetFortniteVersionEasy()
{
    const std::wstring engine_version = GetEngineVersion().c_str();

    try
    {
        float version = std::stof(engine_version.substr(34, 4));
        if (static_cast<int>(version) < 0)
        {
            version = std::stof(engine_version.substr(35, 4));
        }

        return version;
    }
    catch (std::invalid_argument& e)
    {
        return 0.f;
    }
}

inline float GetEngineVersionEasy()
{
    const std::wstring engine_version = GetEngineVersion().c_str();

    // Special case for 2.5
    static float FNVersion = GetFortniteVersionEasy();

    if (FNVersion == 2.5)
    {
        return 419;
    }
    
    return std::stod(engine_version.substr(0, 4)) * 100;
}

inline int GetCL()
{
    std::wstring engine_version = GetEngineVersion().c_str();

    const size_t Position_Underscore = engine_version.find(L"-");
    const size_t Position_Plus = engine_version.find(L"+");
    
    if (Position_Underscore != std::wstring::npos && Position_Plus != std::wstring::npos)
    {
        engine_version = engine_version.substr(Position_Underscore + 1, Position_Plus - (Position_Underscore + 1));

        try
        {
            return std::stoi(engine_version);
        }
        catch (std::invalid_argument& e)
        {
            return 0;
        }
    }

    return 0;
}

class UFortPlaylistAthena* GetPlaylistEasy();

template <typename T>
T* CreateDefaultObject()
{
    if (T::StaticClass())
    {
        return reinterpret_cast<T*>(CreateDefaultObject_Internal(T::StaticClass()));
    }

    return nullptr;
}

template <typename T>
T* CreateDefaultObject(UObject* InClass)
{
    if (InClass)
    {
        return reinterpret_cast<T*>(CreateDefaultObject_Internal(InClass));
    }

    return nullptr;
}

