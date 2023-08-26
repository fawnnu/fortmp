#pragma once

class AFortAthenaSupplyDrop : public AActor
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortAthenaSupplyDrop")

    FVector FindGroundLocationAt(FVector InLocation)
    {
        STATIC_FINDFUNCTION(FindGroundLocationAt)

        return Call<FVector>(Function_FindGroundLocationAt, InLocation);
    }
};

class AAthenaSupplyDrop_Llama_C : public AFortAthenaSupplyDrop
{
public:
    DEFINE_STATIC_CLASS(L"/Game/Athena/SupplyDrops/Llama/AthenaSupplyDrop_Llama.AthenaSupplyDrop_Llama_C")
};