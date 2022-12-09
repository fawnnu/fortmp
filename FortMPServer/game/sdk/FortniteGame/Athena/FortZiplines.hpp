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
};