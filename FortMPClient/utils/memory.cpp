#include "include.hpp"
#include "memory.hpp"

#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

uintptr_t memory_manager::find_pattern(const char* signature, bool bRelative, uint32_t offset)
{
	uintptr_t base_address = reinterpret_cast<uintptr_t>(GetModuleHandle(NULL));
	static auto patternToByte = [](const char* pattern)
	{
		auto bytes = std::vector<int>{};
		const auto start = const_cast<char*>(pattern);
		const auto end = const_cast<char*>(pattern) + strlen(pattern);

		for (auto current = start; current < end; ++current)
		{
			if (*current == '?')
			{
				++current;
				if (*current == '?')
					++current;
				bytes.push_back(-1);
			}
			else { bytes.push_back(strtoul(current, &current, 16)); }
		}
		return bytes;
	};

	const auto dosHeader = (PIMAGE_DOS_HEADER)base_address;
	const auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)base_address + dosHeader->e_lfanew);

	const auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
	auto patternBytes = patternToByte(signature);
	const auto scanBytes = reinterpret_cast<std::uint8_t*>(base_address);

	const auto s = patternBytes.size();
	const auto d = patternBytes.data();

	for (auto i = 0ul; i < sizeOfImage - s; ++i)
	{
		bool found = true;
		for (auto j = 0ul; j < s; ++j)
		{
			if (scanBytes[i + j] != d[j] && d[j] != -1)
			{
				found = false;
				break;
			}
		}
		if (found)
		{
			uintptr_t address = reinterpret_cast<uintptr_t>(&scanBytes[i]);
			if (bRelative)
			{
				address = ((address + offset + 4) + *(int32_t*)(address + offset));
				return address;
			}
			return address;
		}
	}

	return NULL;
}

uintptr_t memory_manager::find_function_by_string(const wchar_t* String, std::vector<DWORD> Opcodes, int Offset, bool Backwards)
{
    __int64 BaseAddress = (__int64)GetModuleHandle(NULL);

	bool bFound = false;
		
	PIMAGE_NT_HEADERS64 Headers = ImageNtHeader(HINSTANCE(BaseAddress));

	//The first section is always .text, the second is always .rdata
	PIMAGE_SECTION_HEADER TextSection = IMAGE_FIRST_SECTION(Headers); 
	PIMAGE_SECTION_HEADER DataSection = IMAGE_FIRST_SECTION(Headers) + 1; 
	
	auto Bytes = reinterpret_cast<uint8_t*>(BaseAddress + TextSection->VirtualAddress);

	for (DWORD i = 0; i < TextSection->Misc.VirtualSize; i++)
	{
		if ((Bytes[i] == 0x4C && Bytes[i+1] == 0x8D) ||
			(Bytes[i] == 0x48 && Bytes[i+1] == 0x8D)) //Opcodes for a string reference (lea)
		{
			//now get the actual address of the string
			auto StringAddress = (__int64(&Bytes[i]) + 7) + *(__int32*)(__int64(&Bytes[i]) + 3);
			
			if ((StringAddress < BaseAddress + DataSection->VirtualAddress) ||
				(StringAddress > BaseAddress + DataSection->VirtualAddress + DataSection->Misc.VirtualSize))
			{
				continue;
			}

			if (wcsstr((const wchar_t*)(StringAddress), String))
			{
				Bytes = &Bytes[i];
				bFound = true;
				break;
			}
		}
	}
	
	if (!bFound) return 0;

	if (Opcodes.size() != 0)
	{
		for (auto Addr = __int64(Bytes)+1;
			Backwards ? Addr > BaseAddress + TextSection->VirtualAddress : Addr < BaseAddress + TextSection->VirtualAddress + TextSection->Misc.VirtualSize;
			Backwards ? Addr-- : Addr++)
		{
			auto Byte = reinterpret_cast<uint8_t*>(Addr);

			for (auto i = 0; i < Opcodes.size(); i++)
			{
				if (Byte[i] == Opcodes[i] || Opcodes[i] == 0x100)
				{
					if (i+1 == Opcodes.size())
					{
						return Addr + Offset + 4 + *(__int32*)(Addr + Offset);
					}
				}
				else
				{
					break;
				}
			}
		}
	}

	DWORD64 ImgBase = 0;
	PRUNTIME_FUNCTION Function = nullptr;
	for (auto Func = RtlLookupFunctionEntry(__int64(Bytes), &ImgBase, NULL); Func; Func = RtlLookupFunctionEntry(BaseAddress + (Func->BeginAddress-1), &ImgBase, NULL))
	{
		Function = Func;
	}

	return Function ? BaseAddress + Function->BeginAddress : 0;
}

bool memory_manager::is_bad_read_pointer(void* pointer_to_read)
{
    if (pointer_cache.find(pointer_to_read) != pointer_cache.end())
	{
		return pointer_cache[pointer_to_read];
	}

	MEMORY_BASIC_INFORMATION mbi;
		
	if (VirtualQuery(pointer_to_read, &mbi, sizeof(mbi)))
	{
		auto mask = (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
		auto is_bad_read = !(mbi.Protect & mask);
		
		if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS))
		{
			is_bad_read = true;
		}

		pointer_cache[pointer_to_read] = is_bad_read;

		return is_bad_read;
	}

	pointer_cache[pointer_to_read] = true;
	
	return true;
}

