#pragma once

template<class T>
struct TArray
{
    TArray() :
        Data(nullptr), Count(0), Max(0)
    {
    }

    T& operator[](int ArrayIdx)
    {
        return Data[ArrayIdx];
    }

    T* begin()
    {
        return reinterpret_cast<T*>(Count, Data);
    }

    T* end()
    {
        return reinterpret_cast<T*>(Count, Data + Count);
    }
    
    int Num() const
    {
        return Count;
    }

    bool Find(T ElementToFind) const
    {
        for (T Element : *this)
        {
            if (ElementToFind == Element)
            {
                return true;
            }
        }

        return false;
    }

    // Pointer to array data
    T* Data;

    // Number of elements in array
    int Count;

    // Maximum number of elements in array
    int Max;
};
