#pragma once

// Reallocate an area of memory
inline void* (*FMemory_Realloc)(PVOID Block, SIZE_T NewSize, int Alignment);

// Remove an element from an array
inline void (*GenericArray_Remove)(PVOID TargetArray, PVOID Property, int ItemIndex);

struct FGuid
{
    int A;
    int B;
    int C;
    int D;

    FORCEINLINE bool operator==(FGuid cmp)
    {
        return this->A == cmp.A && this->B == cmp.B && this->C == cmp.C && this->D == cmp.D;
    };
};

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

    bool Find(T ElementToFind)
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

    int Slack()
    {
        return Max - Count;
    }

    void Reserve(int NumElements)
    {
        Data = Slack() >= NumElements ? Data : (T*)FMemory_Realloc(Data, (Max = Count + NumElements) * sizeof(T), 0);
    }

    void Add(T& InElement)
    {
        Reserve(1);
        Data[Count] = InElement;
        ++Count;
    }

    // Pointer to array data
    T* Data;

    // Number of elements in array
    int Count;

    // Maximum number of elements in array
    int Max;
};

class UObject;

template <typename T>
union TSparseArrayElementOrFreeListLink
{
    T ElementData;

    struct
    {
        int32_t PrevFreeIndex;
        int32_t NextFreeIndex;
    };
};

template <typename T>
class TSparseArray
{
public:
    typedef TSparseArrayElementOrFreeListLink<T> ElementOrFreeListlist;

    TArray<ElementOrFreeListlist> Data;

    char pad[0x28];

    FORCEINLINE T* begin()
    {
        return &Data.begin()->ElementData;
    }

    FORCEINLINE T* end()
    {
        return &Data.end()->ElementData;
    }
};

template <typename T>
class TSetElement
{
public:
    T Element;
    char pad[0x8];
};

template <typename T>
class TSet
{
public:
    typedef TSetElement<T> SetElementType;

    TSparseArray<SetElementType> Elements;

    char pad[0x14];

    FORCEINLINE SetElementType* begin()
    {
        return Elements.begin();
    }

    FORCEINLINE SetElementType* end()
    {
        return Elements.end();
    }
};

template <typename T, typename T1>
class TPair
{
public:
    T Key;
    T1 Value;
};

template <typename Key, typename Value>
class TMap
{
public:
    typedef TPair<Key, Value> MapElementType;

    TSet<MapElementType> Elements;

    FORCEINLINE TSetElement<MapElementType>* begin()
    {
        return Elements.begin();
    }

    FORCEINLINE TSetElement<MapElementType>* end()
    {
        return Elements.end();
    }

    FORCEINLINE MapElementType& operator[](Key key)
    {
        return GetByKey(key);
    };

    FORCEINLINE MapElementType& GetByKey(Key key)
    {
        for (auto Element : *this)
        {
            if (Element.Element.Key == key)
            {
                return Element.Element;
            }
        }
    }
};

struct Struct
{
    // Empty struct used when iterating through dynamic struct arrays
};

template <typename T = Struct>
class TArrayManager
{
private:
    class Iterator
    {
    public:
        Iterator(uintptr_t Address, size_t pSize)
        {
            Element = Address;
            Size = pSize;
        }
        
        T& operator*() const
        {
            return *reinterpret_cast<T*>(Element);
        }

        Iterator& operator++()
        {
            Element += Size;
            return *this;
        }

        bool operator!=(Iterator& cmp)
        {
            return Element != cmp.Element;
        }
    
        uintptr_t Element;
        size_t Size;
    };
    
public:
    TArrayManager(PVOID ArrayPointer, size_t ArraySize)
    {
        Array = ArrayPointer;
        ElementSize = ArraySize;
    }

    FORCEINLINE T& operator[](int Idx)
    {
        auto& TArr = *reinterpret_cast<TArray<Struct>*>(Array);

        return *reinterpret_cast<T*>(uintptr_t(TArr.Data) + Idx * ElementSize);
    }

    int Add(PVOID NewElement)
    {
        auto& TArr = *reinterpret_cast<TArray<Struct>*>(Array); 
        
        TArr.Data = (Struct*)FMemory_Realloc(TArr.Data, ElementSize * (TArr.Count + 1), 0);

        memcpy(TArr.Data + TArr.Count++ * ElementSize, NewElement, ElementSize);

        TArr.Max = TArr.Count;

        return TArr.Count - 1;
    }
    
    void Remove(int Idx, void* ArrayProperty)
    {
        GenericArray_Remove(Array, ArrayProperty, Idx);
    }

    int Num()
    {
        return reinterpret_cast<TArray<Struct>*>(Array)->Num();
    }
    
    Iterator begin()
    {
        return Iterator(uintptr_t(reinterpret_cast<TArray<Struct>*>(Array)->Data), ElementSize);
    }

    Iterator end()
    {
        auto& TArr = *reinterpret_cast<TArray<Struct>*>(Array);
        return Iterator(uintptr_t(TArr.Data) + TArr.Count * ElementSize, ElementSize);
    }

private:
    PVOID Array;
    size_t ElementSize;
};