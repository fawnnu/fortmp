#pragma once

typedef int64_t int64;
typedef uint64_t uint64;
typedef intptr_t intptr;
typedef uintptr_t uintptr;

class engine_manager
{
public:
    engine_manager();
    
    //Core functions that FMP needs to have initialized and usable
    void (*free_internal)(void*);

    void (*fname_to_string)(struct FName*, struct FString&);

    class UObject* (*find_function_by_name)(class UObject*, struct FName, int);
    
    void* (*process_event)(UObject* Object, UObject* Function, void* Params);
    
    class UObject* (*static_find_object)(UObject* Class, void* Outer, const wchar_t* Name, bool ExactClass);
    
    class UObject* (*static_load_object)(UObject*, UObject*, const wchar_t*, const wchar_t*, int, void*, bool);
     
    class UObject* (*spawn_actor)(UObject* World, UObject* Class, struct FVector* Location, struct FRotator* Rotation, const struct FActorSpawnParameters& Params);
    
    struct FString (*get_engine_version)();

    void (*fname_fname)(FName*, const wchar_t*, int);

    class UWorld** GWorld;
};

inline engine_manager* global_engine_manager;

#define check(Pointer)\
    if (Pointer == nullptr)\
    {\
        spdlog::error("{} is invalid.", #Pointer);\
        return;\
    }

#define DEFINE_UCLASS(ClassName, ClassPath, ...)\
    std::string ClassName##_Path = ClassPath;\
    class ClassName : __VA_ARGS__

#define DEFINE_UPROPERTY(Type, Name, ClassName)\
    Type##& Name##()\
    {\
        std::string object_to_find_string = ClassName##_Path + "." + #Name;\
        std::wstring object_to_find = std::wstring(object_to_find_string.begin(), object_to_find_string.end());\
        static int PropertyOffset = *reinterpret_cast<int*>(uintptr(global_engine_manager->static_find_object(nullptr, nullptr, object_to_find.c_str(), false)) + 0x44);\
        return *reinterpret_cast<Type*>(uintptr(this) + PropertyOffset);\
    }