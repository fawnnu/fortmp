#pragma once

struct FByteBulkData
{
    void* Lock(int LockFlags)
    {
        static uintptr_t LockAddress = memory_manager::find_function_by_string(L"Unknown lock flag %i");

        if (LockAddress)
        {
            return reinterpret_cast<void* (*)(void* Self, int)>(LockAddress)(this, LockFlags);
        }
        
        return nullptr;
    }

    void Unlock()
    {
        static uintptr_t UnlockAddress = memory_manager::find_function_by_string(L"STAT_UBD_Unlock");

        if (UnlockAddress)
        {
            reinterpret_cast<void (*)(void* Self)>(UnlockAddress)(this);
        }
        else
        {
            spdlog::error(L"Failed to call FByteBulkData::Unlock, function not found.");
        }
    }
};

struct FTexture2DMipMap
{
    int32_t SizeX;
    
    int32_t SizeY;

    FByteBulkData BulkData;
};

struct FTexturePlatformData
{
    int SizeX;
    int SizeY;

    bool TryLoadMips(int FirstMipToLoad, void** OutMipData)
    {
        if (::TryLoadMips == nullptr)
        {
            spdlog::error(L"Failed to call TryLoadMips: function not found.");
            
            return false;
        }
        
        if (::TryLoadMips(this, FirstMipToLoad, OutMipData))
        {
            return true;
        }
        
        return false;
    }
    
    DEFINE_STRUCT_CLASS(L"/Script/Engine.TexturePlatformData");
};

class UTexture : public UObject
{
public:
    
};

class UTexture2D : public UObject
{
public:
    FTexturePlatformData* PlatformData()
    {
        int PlatformData_Offset = 0;

        if (GetEngineVersionEasy() < 419)
        {
            static void* native_property = FindPropertyByName(this->Class, L"AddressY");                                          
            static int native_property_offset = *reinterpret_cast<int*>(__int64(native_property) + CoreOffsets::Offset_Internal);

            PlatformData_Offset = native_property_offset + 1;
        }
        else if (GetEngineVersionEasy() < 423)
        {
            static void* native_property = FindPropertyByName(this->Class, L"ForceMipLevelsToBeResidentTimestamp");                                          
            static int native_property_offset = *reinterpret_cast<int*>(__int64(native_property) + CoreOffsets::Offset_Internal);

            PlatformData_Offset = native_property_offset + 8;
        }
        else
        {
            static void* native_property = FindPropertyByName(this->Class, L"ImportedSize");                                          
            static int native_property_offset = *reinterpret_cast<int*>(__int64(native_property) + CoreOffsets::Offset_Internal);

            PlatformData_Offset = native_property_offset + 12;
        }
        
        return *reinterpret_cast<FTexturePlatformData**>(__int64(this) + PlatformData_Offset);   
    }
};