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


/*
 * TArrayManager: A custom implementation to deal with TArrays completely dynamicallyà
 * Uses pointers and sizes to iterate and access elements of a TArray
 */

template<typename T>
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
    TArrayManager(TArray<T>& in_array_reference, DWORD in_size)
        : local_array_reference(in_array_reference), local_element_size(in_size)
    {
        
    }

    FORCEINLINE T& operator[](int Idx)
    {
        return *reinterpret_cast<T*>(uintptr_t(local_array_reference.Data) + Idx * local_element_size);
    }

    int Add(PVOID NewElement)
    {
        local_array_reference.Data = (T*)FMemory_Realloc(local_array_reference.Data, local_element_size * (local_array_reference.Num() + 1), 0);

        memcpy(local_array_reference.Data + local_array_reference.Count++ * local_element_size, NewElement, local_element_size);

        local_array_reference.Max = local_array_reference.Count;

        return local_array_reference.Count - 1;
    }
    
    void Remove(int Idx, void* ArrayProperty)
    {
        GenericArray_Remove(&local_array_reference, ArrayProperty, Idx);
    }

    int Num()
    {
        return local_array_reference.Num();
    }

    Iterator begin()
    {
        return Iterator(uintptr_t(local_array_reference.Data), local_element_size);
    }

    Iterator end()
    {
        return Iterator(uintptr_t(local_array_reference.Data) + local_array_reference.Num() * local_element_size, local_element_size);
    }
    
private:
    TArray<T>& local_array_reference;
    DWORD local_element_size;
};