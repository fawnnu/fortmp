#pragma once

struct FVector
{
    FVector() = default;
    
    /*
    FVector(float InX, float InY, float InZ) :
        X(InX), Y(InY), Z(InZ)
    {
    }
    */
    
    // Vector's X component
    float X;

    // Vector's Y component
    float Y;

    // Vector's Z component
    float Z;
};

struct FRotator
{
    FRotator() = default;
    
    FRotator(float InPitch, float InYaw, float InRoll) :
        Pitch(InPitch), Yaw(InYaw), Roll(InRoll)
    {
    }
    
    // Rotation around the Y axis
    float Pitch;

    // Rotation around the X axis
    float Yaw;

    // Rotation around the Z axis
    float Roll;
};

struct alignas(16) FQuat
{
    /*
    FQuat(float InX, float InY, float InZ, float InW) :
        X(InX), Y(InY), Z(InZ), W(InW)
    {
    }
    */
    
    // Quaternion's X component
    float X;

    // Quaternion's Y component
    float Y;

    // Quaternion's Z component
    float Z;

    // Quaternion's W component
    float W;
};

struct alignas(16) FTransform
{
    // Rotation of the transformation
    FQuat Rotation;

    // Translation of this transformation
    FVector Translation;
    
    char Unk00[0x4];

    // 3D scale as a vector
    FVector Scale3D;
    
    char Unk01[0x4];
};