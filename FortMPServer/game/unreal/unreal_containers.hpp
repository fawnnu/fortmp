#pragma once

// Reallocate an area of memory
inline void* (*FMemory_Realloc)(PVOID Block, SIZE_T NewSize, int Alignment);

// Remove an element from an array
inline void (*GenericArray_Remove)(PVOID TargetArray, PVOID Property, int ItemIndex);

struct FGuid
{
	FGuid() = default;
	
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
	
	T& operator[](int ArrayIdx) const
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

	void Add(const T& InElement)
	{
		Reserve(1);
		Data[Count] = InElement;
		++Count;
	}

	std::vector<T> ToVector()
	{
		std::vector<T> VectorizedTArray;
		VectorizedTArray.assign(Data, Data + Count);

		return VectorizedTArray;
	}

	void Free();
    
	// Pointer to array data
	T* Data;

	// Number of elements in array
	int Count;

	// Maximum number of elements in array
	int Max;
};

template <typename... Types>
struct TOr;

	template <bool LHSValue, typename... RHS>
	struct TOrValue
	{
		enum { Value = TOr<RHS...>::Value };
	};

	template <typename... RHS>
	struct TOrValue<true, RHS...>
	{
		enum { Value = true };
	};

	template <typename LHS, typename... RHS>
	struct TOr<LHS, RHS...> : TOrValue<LHS::Value, RHS...>
	{
	};

	template <>
	struct TOr<>
	{
		enum { Value = false };
	};

	/**
	 * Does a boolean NOT of the ::Value static members of the type.
	 */
	template <typename Type>
	struct TNot
	{
		enum { Value = !Type::Value };
	};


	template <typename T>
	struct TIsPointer
	{
		enum { Value = false };
	};

	template <typename T> struct TIsPointer<               T*> { enum { Value = true }; };
	template <typename T> struct TIsPointer<const          T*> { enum { Value = true }; };
	template <typename T> struct TIsPointer<      volatile T*> { enum { Value = true }; };
	template <typename T> struct TIsPointer<const volatile T*> { enum { Value = true }; };

	template <typename T> struct TIsPointer<const          T> { enum { Value = TIsPointer<T>::Value }; };
	template <typename T> struct TIsPointer<      volatile T> { enum { Value = TIsPointer<T>::Value }; };
	template <typename T> struct TIsPointer<const volatile T> { enum { Value = TIsPointer<T>::Value }; };

	template <typename T>
	struct TIsArithmetic
	{
		enum { Value = false };
	};

	template <> struct TIsArithmetic<float> { enum { Value = true }; };
	template <> struct TIsArithmetic<double> { enum { Value = true }; };
	template <> struct TIsArithmetic<long double> { enum { Value = true }; };
	template <> struct TIsArithmetic<uint8_t> { enum { Value = true }; };
	template <> struct TIsArithmetic<uint16_t> { enum { Value = true }; };
	template <> struct TIsArithmetic<uint32_t> { enum { Value = true }; };
	template <> struct TIsArithmetic<int8_t> { enum { Value = true }; };
	template <> struct TIsArithmetic<int16_t> { enum { Value = true }; };
	template <> struct TIsArithmetic<int32_t> { enum { Value = true }; };
	template <> struct TIsArithmetic<bool> { enum { Value = true }; };

	template <typename T> struct TIsArithmetic<const          T> { enum { Value = TIsArithmetic<T>::Value }; };
	template <typename T> struct TIsArithmetic<      volatile T> { enum { Value = TIsArithmetic<T>::Value }; };
	template <typename T> struct TIsArithmetic<const volatile T> { enum { Value = TIsArithmetic<T>::Value }; };

	template <bool Predicate, typename Result = void>
	class TEnableIf;

	template <typename Result>
	class TEnableIf<true, Result>
	{
	public:
		typedef Result Type;
	};

	template <typename Result>
	class TEnableIf<false, Result>
	{ };

	template <typename T>
	struct TUseBitwiseSwap
	{
		// We don't use bitwise swapping for 'register' types because this will force them into memory and be slower.
		enum { Value = !TOrValue<__is_enum(T), TIsPointer<T>, TIsArithmetic<T>>::Value };
	};

	template <typename T> struct TRemoveReference { typedef T Type; };
	template <typename T> struct TRemoveReference<T& > { typedef T Type; };
	template <typename T> struct TRemoveReference<T&&> { typedef T Type; };

	template <typename T>
	FORCEINLINE typename TRemoveReference<T>::Type&& MoveTemp(T&& Obj)
	{
		return (typename TRemoveReference<T>::Type&&)Obj;
	}

	template <typename T>
	inline typename TEnableIf<!TUseBitwiseSwap<T>::Value>::Type Swap(T& A, T& B)
	{
		T Temp = MoveTemp(A);
		A = MoveTemp(B);
		B = MoveTemp(Temp);
	}

	template <typename T>
	inline void Exchange(T& A, T& B)
	{
		Swap(A, B);
	}

	template<class T, class PREDICATE_CLASS>
	void SortInternal(T* First, const int32_t Num, const PREDICATE_CLASS& Predicate)
	{
		struct FStack
		{
			T* Min;
			T* Max;
		};

		if (Num < 2)
		{
			return;
		}
		FStack RecursionStack[32] = { {First,First + Num - 1} }, Current, Inner;
		for (FStack* StackTop = RecursionStack; StackTop >= RecursionStack; --StackTop) //-V625
			{
			Current = *StackTop;
			Loop:
				int Count = Current.Max - Current.Min + 1;
			if (Count <= 8)
			{
				// Use simple bubble-sort.
				while (Current.Max > Current.Min)
				{
					T* Max, * Item;
					for (Max = Current.Min, Item = Current.Min + 1; Item <= Current.Max; Item++)
					{
						if (Predicate(*Max, *Item))
						{
							Max = Item;
						}
					}
					Exchange(*Max, *Current.Max--);
				}
			}
			else
			{
				// Grab middle element so sort doesn't exhibit worst-cast behavior with presorted lists.
				Exchange(Current.Min[Count / 2], Current.Min[0]);

				// Divide list into two halves, one with items <=Current.Min, the other with items >Current.Max.
				Inner.Min = Current.Min;
				Inner.Max = Current.Max + 1;
				for (; ; )
				{
					while (++Inner.Min <= Current.Max && !Predicate(*Current.Min, *Inner.Min));
					while (--Inner.Max > Current.Min && !Predicate(*Inner.Max, *Current.Min));
					if (Inner.Min > Inner.Max)
					{
						break;
					}
					Exchange(*Inner.Min, *Inner.Max);
				}
				Exchange(*Current.Min, *Inner.Max);

				// Save big half and recurse with small half.
				if (Inner.Max - 1 - Current.Min >= Current.Max - Inner.Min)
				{
					if (Current.Min + 1 < Inner.Max)
					{
						StackTop->Min = Current.Min;
						StackTop->Max = Inner.Max - 1;
						StackTop++;
					}
					if (Current.Max > Inner.Min)
					{
						Current.Min = Inner.Min;
						goto Loop;
					}
				}
				else
				{
					if (Current.Max > Inner.Min)
					{
						StackTop->Min = Inner.Min;
						StackTop->Max = Current.Max;
						StackTop++;
					}
					if (Current.Min + 1 < Inner.Max)
					{
						Current.Max = Inner.Max - 1;
						goto Loop;
					}
				}
			}
			}
	}

	template<typename T, class PREDICATE_CLASS>
	struct TDereferenceWrapper
	{
		const PREDICATE_CLASS& Predicate;

		TDereferenceWrapper(const PREDICATE_CLASS& InPredicate)
			: Predicate(InPredicate) {}

		/** Pass through for non-pointer types */
		FORCEINLINE bool operator()(T& A, T& B) { return Predicate(A, B); }
		FORCEINLINE bool operator()(const T& A, const T& B) const { return Predicate(A, B); }
	};

	/** Partially specialized version of the above class */
	template<typename T, class PREDICATE_CLASS>
	struct TDereferenceWrapper<T*, PREDICATE_CLASS>
	{
		const PREDICATE_CLASS& Predicate;

		TDereferenceWrapper(const PREDICATE_CLASS& InPredicate)
			: Predicate(InPredicate) {}

		/** Dereference pointers */
		FORCEINLINE bool operator()(T* A, T* B) const
		{
			return Predicate(*A, *B);
		}
	};

	template<class T, class PREDICATE_CLASS>
	static void Sort(T** First, const int32_t Num, const PREDICATE_CLASS& Predicate)
	{
		SortInternal(First, Num, TDereferenceWrapper<T*, PREDICATE_CLASS>(Predicate));
	}

template <typename T>
struct TIndirectArray
{
    typedef TArray<void*> InternalArrayType;

    FORCEINLINE T& operator[](int32_t Index)
    {
        return *static_cast<T*>(Array[Index]);
    }
    
    InternalArrayType Array;
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

	FORCEINLINE bool Contains(Key key)
    {
    	for (auto Element : *this)
    	{
    		if (Element.Element.Key == key)
    		{
    			return true;
    		}
    	}
    	return false;
    }
};

struct Struct
{
    // Empty struct used when iterating through dynamic struct arrays
};

template <typename T>
class TStaticArrayManager
{
public:
	FORCEINLINE T& operator[](int Idx)
	{
		return *reinterpret_cast<T*>(uintptr_t(this) + Idx * sizeof(T));
	}
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

	TArrayManager(TArray<T>& ArrayRef)
    {
	    Array = &ArrayRef;
    	ElementSize = T::GetStructSize();
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

	void Clear(void* ArrayProperty)
	{
    	if (ArrayProperty)
    	{
    		for (int i = Num() - 1; i >= 0; i--)
    		{
    			Remove(i, ArrayProperty);
    		}
    	}
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