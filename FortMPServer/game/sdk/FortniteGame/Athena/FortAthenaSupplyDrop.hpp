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

class AFortAthenaSupplyDropBalloon : public AActor
{
public:
    DEFINE_STATIC_CLASS(L"/Script/FortniteGame.FortAthenaSupplyDropBalloon");
};

class AAthenaSupplyDrop_Llama_C : public AFortAthenaSupplyDrop
{
public:
    DEFINE_CLASS(L"/Game/Athena/SupplyDrops/Llama/AthenaSupplyDrop_Llama.AthenaSupplyDrop_Llama_C")
};

class AAthenaSupplyDrop_C : public AFortAthenaSupplyDrop
{
public:
    DEFINE_CLASS(L"/Game/Athena/SupplyDrops/AthenaSupplyDrop.AthenaSupplyDrop_C")

    DEFINE_PROPERTY(bool, bLanded);
    DEFINE_PROPERTY(FVector, GroundLocation);
};