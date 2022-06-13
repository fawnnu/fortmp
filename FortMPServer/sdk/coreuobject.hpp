#pragma once

class UObject
{
public:
    void** VTableObject;
    int ObjectFlags;
    int InternalIndex;
    UObject* Class;
    FName NamePrivate;
    UObject* Outer;

    std::wstring get_name()
    {
        return NamePrivate.ToString();
    }

    std::wstring get_full_name()
    {
        std::wstring result;
        
        for (UObject* outer = Outer; outer; outer = outer->Outer)
        {
            result = outer->get_name() + L"." + result;
        }

        result = result + this->get_name();

        return result;
    }

    bool is_a(UObject* ComparisonClass) const
    {
        return Class == ComparisonClass;
    }

    template <typename T = char, typename ...Params>
    T call_function(const wchar_t* function_name, Params ... args)
    {
        int16 parms_size = 0;
        int16 return_offset = 0;
        
        std::vector<int> offset_vector;
        char offset_vector_iterator = 0;
        
        if (UObject* function_object = global_engine_manager->find_function_by_name(this->Class, function_name, 1))
        {
            for (UObject* Property = *reinterpret_cast<UObject**>(uintptr(function_object) + 0x38); Property; Property = *reinterpret_cast<UObject**>(uintptr(Property) + 0x28))
            {
                int offset = *reinterpret_cast<int*>(uintptr(Property) + 0x44);
            
                parms_size = offset + // Offset_Internal
                    *reinterpret_cast<int*>(uintptr(Property) + 0x34); // ElementSize

                offset_vector.push_back(offset);
            
                if (*reinterpret_cast<uint64*>(uintptr(Property) + 0x38) & 0x0000000000000400) //CPF_ReturnParm
                {
                    return_offset = offset;
                }
            }

            char* params = (char*)alloca(parms_size);

            std::apply([&](auto ... argument)
                {
                    ((
                        memcpy(params + offset_vector[offset_vector_iterator++], argument, sizeof(*argument))
                    ), ...);
                }, std::make_tuple(&args...));
        
            global_engine_manager->process_event(this, function_object, params);

            return *reinterpret_cast<T*>(uintptr(params) + return_offset);
        }
    }

private:
    
};

DEFINE_UCLASS(UFunction, "/Script/CoreUObject.Function", public UObject)
{
    public:

    GENERATE_STATIC_CLASS(UFunction);
    
    // returns native func pointer
    void* get_func()
    {
        // offset of native function in a UFunction struct
        static int offset = *reinterpret_cast<int*>(uintptr(StaticClass()) + 0x40) - sizeof(void*);

        return *reinterpret_cast<void**>(uintptr(this) + offset);
    }
};