#pragma once

struct FVector
{
    FVector() = default;
    
    FVector(float InX, float InY, float InZ) :
        X(InX), Y(InY), Z(InZ)
    {
    }

    FORCEINLINE FVector operator-(const FVector& V) const
    {
        return FVector(X - V.X, Y - V.Y, Z - V.Z);
    }

    FORCEINLINE float operator|(const FVector& V) const
    {
        return X*V.X + Y*V.Y + Z*V.Z;
    }

    FORCEINLINE bool operator==(const FVector& V) const
    {
        return X==V.X && Y==V.Y && Z==V.Z;
    }

    FORCEINLINE float SizeSquared() const
    {
        return X*X + Y*Y + Z*Z;
    }

    FORCEINLINE bool IsValid() const
    {
        return X != 0 && Y != 0 && Z != 0;
    }

    FORCEINLINE std::string ToAnsiString() const
    {
        return std::to_string(X) + " " + std::to_string(Y) + " " + std::to_string(Z);
    }

    FORCEINLINE std::wstring ToUnicodeString() const
    {
        return std::to_wstring(X) + L" " + std::to_wstring(Y) + L" " + std::to_wstring(Z);
    }
        
    // Vector's X component
    float X;

    // Vector's Y component
    float Y;

    // Vector's Z component
    float Z;
};

struct FColor
{
    FColor() = default;
    
    FColor(float InR, float InG, float InB, float InA) :
        R(InR), G(InG), B(InB), A(InA)
    {
    }
    
    float R;
    
    float G;

    float B;

    float A;
};

struct FLinearColor
{
    FLinearColor() = default;
    
    FLinearColor(float InR, float InG, float InB, float InA) :
        R(InR), G(InG), B(InB), A(InA)
    {
    }
    
    float R;
    
    float G;

    float B;

    float A;
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

struct FGenericPlatformMath
{
    /**
     * Converts a float to an integer with truncation towards zero.
     * @param F		Floating point value to convert
     * @return		Truncated integer.
     */
    static constexpr FORCEINLINE int32_t TruncToInt(float F)
    {
        return (int32_t)F;
    }

    /**
     * Converts a float to an integer value with truncation towards zero.
     * @param F		Floating point value to convert
     * @return		Truncated integer value.
     */
    static constexpr FORCEINLINE float TruncToFloat(float F)
    {
        return (float)TruncToInt(F);
    }

    /**
     * Converts a float to a nearest less or equal integer.
     * @param F		Floating point value to convert
     * @return		An integer less or equal to 'F'.
     */
    static FORCEINLINE int32_t FloorToInt(float F)
    {
        return TruncToInt(floorf(F));
    }

    /**
    * Converts a float to the nearest less or equal integer.
    * @param F		Floating point value to convert
    * @return		An integer less or equal to 'F'.
    */
    static FORCEINLINE float FloorToFloat(float F)
    {
        return floorf(F);
    }

    /**
    * Converts a double to a less or equal integer.
    * @param F		Floating point value to convert
    * @return		The nearest integer value to 'F'.
    */
    static FORCEINLINE double FloorToDouble(double F)
    {
        return floor(F);
    }

    /**
     * Converts a float to the nearest integer. Rounds up when the fraction is .5
     * @param F		Floating point value to convert
     * @return		The nearest integer to 'F'.
     */
    static FORCEINLINE int32_t RoundToInt(float F)
    {
        return FloorToInt(F + 0.5f);
    }

    template< class T > 
    static constexpr FORCEINLINE T Max( const T A, const T B )
    {
        return (A>=B) ? A : B;
    }

    template< class T > 
    static constexpr FORCEINLINE T Min( const T A, const T B )
    {
        return (A<=B) ? A : B;
    }

    template< class T > 
    static FORCEINLINE T Clamp( const T X, const T Min, const T Max )
    {
        return X<Min ? Min : X<Max ? X : Max;
    }
};

struct FMath : FGenericPlatformMath
{
    template< class T > 
    static FORCEINLINE T Square( const T A )
    {
        return A*A;
    }
};

struct FVector2D
{
    FVector2D() = default;
    
    float X;
    float Y;
};