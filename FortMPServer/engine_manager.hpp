#pragma once

typedef int64_t int64;
typedef uint64_t uint64;
typedef intptr_t intptr;
typedef uintptr_t uintptr;

#define check(Pointer)\
    if (Pointer == nullptr)\
    {\
        spdlog::error("{} is invalid.", #Pointer);\
        return;\
    }
 

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
};

inline engine_manager* global_engine_manager;
