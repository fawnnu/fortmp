#pragma once

class IInterface : public UObject
{
public:
    
};

class IFortInventoryOwnerInterface : public IInterface
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortInventoryOwnerInterface");
};

struct FAssetData
{
    DEFINE_STRUCT_PROPERTY(FName, ObjectPath);  
    DEFINE_STRUCT_PROPERTY(FName, PackageName); 
    DEFINE_STRUCT_PROPERTY(FName, PackagePath); 
    DEFINE_STRUCT_PROPERTY(FName, AssetName);   
    DEFINE_STRUCT_PROPERTY(FName, AssetClass);  

    DEFINE_STRUCT_CLASS(L"/Script/AssetRegistry.AssetData")
};

class IAssetRegistry : public UObject
{
public:
    bool GetAssetsByPath(FName PackagePath, TArray<FAssetData>& OutAssetData, bool bRecursive, bool bIncludeOnlyOnDiskAssets)
    {
        return reinterpret_cast<bool (*)(void*, FName, TArray<FAssetData>&, bool, bool)>(VTable[4])(this, PackagePath, OutAssetData, bRecursive, bIncludeOnlyOnDiskAssets);
    }
};

class UAssetRegistryHelpers : public UObject
{
public:
    static TScriptInterface<IAssetRegistry> GetAssetRegistry()
    {
        static UAssetRegistryHelpers* DefaultObj = FindObject<UAssetRegistryHelpers>(L"/Script/AssetRegistry.Default__AssetRegistryHelpers");

        static UObject* FunctionObject = FindObject(L"/Script/AssetRegistry.AssetRegistryHelpers.GetAssetRegistry");

        return DefaultObj->Call<TScriptInterface<IAssetRegistry>>(FunctionObject);
    }
};