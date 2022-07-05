#pragma once

class ABP_Athena_Environmental_ZipLine_C;

struct FZiplinePawnState
{
    DEFINE_STRUCT_PROPERTY(ABP_Athena_Environmental_ZipLine_C*, Zipline);
    DEFINE_STRUCT_PROPERTY(bool, bIsZiplining);
    DEFINE_STRUCT_PROPERTY(bool, bJumped);
    DEFINE_STRUCT_PROPERTY(int, AuthoritativeValue);
    DEFINE_STRUCT_PROPERTY(FVector, SocketOffset);
    DEFINE_STRUCT_PROPERTY(float, TimeZipliningBegan);
    DEFINE_STRUCT_PROPERTY(float, TimeZipliningEndedFromJump);
    
    DEFINE_STRUCT_CLASS(L"/Script/FortniteGame.ZiplinePawnState");
};

class AFortAthenaZipline : public UObject
{
public:
};

class ABP_Athena_Environmental_ZipLine_C : public AFortAthenaZipline
{
public:
    void StartZipLining(AFortPlayerPawn* FortPlayerPawn)
    {
        Call(L"StartZipLining", FortPlayerPawn);
    }
    
    void ReleaseZipline(AFortPlayerPawn* FortPlayerPawn)
    {
        Call(L"ReleaseZipline", FortPlayerPawn);
    }
    
    void GetCharacterCustomMovementMode(APawn* Character, uint8_t* CustomMovementMode, uint8_t* MovementMode)
    {
        static UFunction* GetCharacterCustomMovementMode_Func = FindObject<UFunction>(L"/Game/Athena/Items/Consumables/ZipLine/BP_Athena_Environmental_ZipLine.BP_Athena_Environmental_ZipLine_C.GetCharacterCustomMovementMode", true);
        
        struct GetCharacterCustomMovementMode_Params
        {
            APawn* Character;

            uint8_t CustomMovementMode; //out parm
            
            uint8_t MovementMode; //out parm
        } function_params;

        function_params.Character = Character;

        ProcessEvent(this, GetCharacterCustomMovementMode_Func, &function_params);

        *CustomMovementMode = function_params.CustomMovementMode;
        *MovementMode = function_params.MovementMode;
    }
};