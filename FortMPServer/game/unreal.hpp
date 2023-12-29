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

// Construct a struct
inline void (*ConstructUScriptStruct)(UObject** OutStruct, struct FStructParams* Params);

// Construct collision parameters
inline void* (*FCollisionQueryParams__Ctor)(struct FCollisionQueryParams*, FName TraceTag, FName StatId, bool bTraceComplex, class AActor* IgnoreActor);

// Construct collision parameters
inline bool (*LineTraceMultiByChannel)(UWorld*, TArray<struct FHitResult>& OutHits, FVector& Start, FVector& End, int TraceChannel, FCollisionQueryParams& CollisionQueryParams, struct FCollisionResponseParams& CollisionResponseParams);

// Add ignored actors to params
inline void (*FCollisionQueryParams__AddIgnoredActors)(FCollisionQueryParams*, TArray<AActor*>&);

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
    inline DWORD Script;
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
    
    template <typename T>
    T* Cast()
    {
        if (IsA(T::StaticClass()))
        {
            return reinterpret_cast<T*>(this);
        }

        return nullptr;
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
public:
    static UObject* StaticClass()
    {
        static UObject* static_class = FindObject(L"/Script/CoreUObject.Property");

        return static_class;
    }
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
    static UObject* StaticClass()
    {
        static UObject* static_class = FindObject(L"/Script/CoreUObject.Function");

        return static_class;
    }
    
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

struct FTimespan
{
    int64_t Ticks;
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

enum EStructFlags
{
    STRUCT_NoFlags                       = 0x00000000,
    STRUCT_Native                        = 0x00000001,
    STRUCT_IdenticalNative               = 0x00000002,
    STRUCT_HasInstancedReference         = 0x00000004,
    STRUCT_NoExport                      = 0x00000008,
    STRUCT_Atomic                        = 0x00000010,
    STRUCT_Immutable                     = 0x00000020,
    STRUCT_AddStructReferencedObjects    = 0x00000040,
    STRUCT_RequiredAPI                   = 0x00000200,
    STRUCT_NetSerializeNative            = 0x00000400,
    STRUCT_SerializeNative               = 0x00000800,
    STRUCT_CopyNative                    = 0x00001000,
    STRUCT_IsPlainOldData                = 0x00002000,
    STRUCT_NoDestructor                  = 0x00004000,
    STRUCT_ZeroConstructor               = 0x00008000,
    STRUCT_ExportTextItemNative          = 0x00010000,
    STRUCT_ImportTextItemNative          = 0x00020000,
    STRUCT_PostSerializeNative           = 0x00040000,
    STRUCT_SerializeFromMismatchedTag    = 0x00080000,
    STRUCT_NetDeltaSerializeNative       = 0x00100000,
    STRUCT_PostScriptConstruct           = 0x00200000,
    STRUCT_NetSharedSerialization        = 0x00400000,
    STRUCT_Trashed                       = 0x00800000,
    STRUCT_Inherit                       = STRUCT_HasInstancedReference|STRUCT_Atomic,
    STRUCT_ComputedFlags                 = STRUCT_NetDeltaSerializeNative | STRUCT_NetSerializeNative | STRUCT_SerializeNative | STRUCT_PostSerializeNative | STRUCT_CopyNative | STRUCT_IsPlainOldData | STRUCT_NoDestructor | STRUCT_ZeroConstructor | STRUCT_IdenticalNative | STRUCT_AddStructReferencedObjects | STRUCT_ExportTextItemNative | STRUCT_ImportTextItemNative | STRUCT_SerializeFromMismatchedTag | STRUCT_PostScriptConstruct | STRUCT_NetSharedSerialization,
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

struct FStructPropertyParams
{
    EPropertyClass   Type;
    const char*      NameUTF8;
    int     ObjectFlags;
    uintptr_t   PropertyFlags;
    int            ArrayDim;
    const char*      RepNotifyFuncUTF8;
    int            Offset;
    UObject* (*ScriptStructFunc)();
};

struct FStructParams
{
    UObject* (*OuterFunc)();
    UObject* (*SuperFunc)();
    void* (*StructOpsFunc)();
    const char* NameUTF8;
    int ObjectFlags;
    int StructFlags;
    SIZE_T SizeOf;
    SIZE_T AlignOf;
    void** PropertyArray;
    int NumProperties;
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

enum EExprToken
{
	// Variable references.
	EX_LocalVariable		= 0x00,	// A local variable.
	EX_InstanceVariable		= 0x01,	// An object variable.
	EX_DefaultVariable		= 0x02, // Default variable for a class context.
	//						= 0x03,
	EX_Return				= 0x04,	// Return from function.
	//						= 0x05,
	EX_Jump					= 0x06,	// Goto a local address in code.
	EX_JumpIfNot			= 0x07,	// Goto if not expression.
	//						= 0x08,
	EX_Assert				= 0x09,	// Assertion.
	//						= 0x0A,
	EX_Nothing				= 0x0B,	// No operation.
	//						= 0x0C,
	//						= 0x0D,
	//						= 0x0E,
	EX_Let					= 0x0F,	// Assign an arbitrary size value to a variable.
	//						= 0x10,
	//						= 0x11,
	EX_ClassContext			= 0x12,	// Class default object context.
	EX_MetaCast             = 0x13, // Metaclass cast.
	EX_LetBool				= 0x14, // Let boolean variable.
	EX_EndParmValue			= 0x15,	// end of default value for optional function parameter
	EX_EndFunctionParms		= 0x16,	// End of function call parameters.
	EX_Self					= 0x17,	// Self object.
	EX_Skip					= 0x18,	// Skippable expression.
	EX_Context				= 0x19,	// Call a function through an object context.
	EX_Context_FailSilent	= 0x1A, // Call a function through an object context (can fail silently if the context is NULL; only generated for functions that don't have output or return values).
	EX_VirtualFunction		= 0x1B,	// A function call with parameters.
	EX_FinalFunction		= 0x1C,	// A prebound function call with parameters.
	EX_IntConst				= 0x1D,	// Int constant.
	EX_FloatConst			= 0x1E,	// Floating point constant.
	EX_StringConst			= 0x1F,	// String constant.
	EX_ObjectConst		    = 0x20,	// An object constant.
	EX_NameConst			= 0x21,	// A name constant.
	EX_RotationConst		= 0x22,	// A rotation constant.
	EX_VectorConst			= 0x23,	// A vector constant.
	EX_ByteConst			= 0x24,	// A byte constant.
	EX_IntZero				= 0x25,	// Zero.
	EX_IntOne				= 0x26,	// One.
	EX_True					= 0x27,	// Bool True.
	EX_False				= 0x28,	// Bool False.
	EX_TextConst			= 0x29, // FText constant
	EX_NoObject				= 0x2A,	// NoObject.
	EX_TransformConst		= 0x2B, // A transform constant
	EX_IntConstByte			= 0x2C,	// Int constant that requires 1 byte.
	EX_NoInterface			= 0x2D, // A null interface (similar to EX_NoObject, but for interfaces)
	EX_DynamicCast			= 0x2E,	// Safe dynamic class casting.
	EX_StructConst			= 0x2F, // An arbitrary UStruct constant
	EX_EndStructConst		= 0x30, // End of UStruct constant
	EX_SetArray				= 0x31, // Set the value of arbitrary array
	EX_EndArray				= 0x32,
	//						= 0x33,
	EX_UnicodeStringConst   = 0x34, // Unicode string constant.
	EX_Int64Const			= 0x35,	// 64-bit integer constant.
	EX_UInt64Const			= 0x36,	// 64-bit unsigned integer constant.
	//						= 0x37,
	EX_PrimitiveCast		= 0x38,	// A casting operator for primitives which reads the type as the subsequent byte
	EX_SetSet				= 0x39,
	EX_EndSet				= 0x3A,
	EX_SetMap				= 0x3B,
	EX_EndMap				= 0x3C,
	EX_SetConst				= 0x3D,
	EX_EndSetConst			= 0x3E,
	EX_MapConst				= 0x3F,
	EX_EndMapConst			= 0x40,
	//						= 0x41,
	EX_StructMemberContext	= 0x42, // Context expression to address a property within a struct
	EX_LetMulticastDelegate	= 0x43, // Assignment to a multi-cast delegate
	EX_LetDelegate			= 0x44, // Assignment to a delegate
	EX_LocalVirtualFunction	= 0x45, // Special instructions to quickly call a virtual function that we know is going to run only locally
	EX_LocalFinalFunction	= 0x46, // Special instructions to quickly call a final function that we know is going to run only locally
	//						= 0x47, // CST_ObjectToBool
	EX_LocalOutVariable		= 0x48, // local out (pass by reference) function parameter
	//						= 0x49, // CST_InterfaceToBool
	EX_DeprecatedOp4A		= 0x4A,
	EX_InstanceDelegate		= 0x4B,	// const reference to a delegate or normal function object
	EX_PushExecutionFlow	= 0x4C, // push an address on to the execution flow stack for future execution when a EX_PopExecutionFlow is executed.   Execution continues on normally and doesn't change to the pushed address.
	EX_PopExecutionFlow		= 0x4D, // continue execution at the last address previously pushed onto the execution flow stack.
	EX_ComputedJump			= 0x4E,	// Goto a local address in code, specified by an integer value.
	EX_PopExecutionFlowIfNot = 0x4F, // continue execution at the last address previously pushed onto the execution flow stack, if the condition is not true.
	EX_Breakpoint			= 0x50, // Breakpoint.  Only observed in the editor, otherwise it behaves like EX_Nothing.
	EX_InterfaceContext		= 0x51,	// Call a function through a native interface variable
	EX_ObjToInterfaceCast   = 0x52,	// Converting an object reference to native interface variable
	EX_EndOfScript			= 0x53, // Last byte in script code
	EX_CrossInterfaceCast	= 0x54, // Converting an interface variable reference to native interface variable
	EX_InterfaceToObjCast   = 0x55, // Converting an interface variable reference to an object
	//						= 0x56,
	//						= 0x57,
	//						= 0x58,
	//						= 0x59,
	EX_WireTracepoint		= 0x5A, // Trace point.  Only observed in the editor, otherwise it behaves like EX_Nothing.
	EX_SkipOffsetConst		= 0x5B, // A CodeSizeSkipOffset constant
	EX_AddMulticastDelegate = 0x5C, // Adds a delegate to a multicast delegate's targets
	EX_ClearMulticastDelegate = 0x5D, // Clears all delegates in a multicast target
	EX_Tracepoint			= 0x5E, // Trace point.  Only observed in the editor, otherwise it behaves like EX_Nothing.
	EX_LetObj				= 0x5F,	// assign to any object ref pointer
	EX_LetWeakObjPtr		= 0x60, // assign to a weak object pointer
	EX_BindDelegate			= 0x61, // bind object and name to delegate
	EX_RemoveMulticastDelegate = 0x62, // Remove a delegate from a multicast delegate's targets
	EX_CallMulticastDelegate = 0x63, // Call multicast delegate
	EX_LetValueOnPersistentFrame = 0x64,
	EX_ArrayConst			= 0x65,
	EX_EndArrayConst		= 0x66,
	EX_SoftObjectConst		= 0x67,
	EX_CallMath				= 0x68, // static pure function from on local call space
	EX_SwitchValue			= 0x69,
	EX_InstrumentationEvent	= 0x6A, // Instrumentation event
	EX_ArrayGetByRef		= 0x6B,
	EX_Max					= 0x100,
};

enum EBlueprintTextLiteralType
{
    Empty,
    LocalizedText,
    InvariantText,
    LiteralString,
    StringTableEntry,
};

struct FScriptName
{
    int32_t ComparisonIndex;
    int32_t DisplayIndex;
    uint32_t Number;

    FName ToName()
    {
        return FName(ComparisonIndex, Number);
    }
};

struct FCollisionResponseParams
{
    FCollisionResponseParams()
    {
        memset(Padding, 0x2, 0x20);
    }
    
    char Padding[0x20];
};

struct FCollisionQueryParams
{
    FCollisionQueryParams()
    {
        
    }
    
    FCollisionQueryParams(FName TagName, bool bTraceComplex, AActor* ActorToIgnore);
    
    char Padding[0x20];
    bool bReturnPhysicalMaterial;
    char Padding_2[0x1DF];

    void AddIgnoredActors(TArray<AActor*>& ActorsToAdd)
    {
        FCollisionQueryParams__AddIgnoredActors(this, ActorsToAdd);
    }
};

struct FText
{
    char Padding[0x18];
};