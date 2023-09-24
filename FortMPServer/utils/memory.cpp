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

uintptr_t memory_manager::find_function_by_string(const wchar_t* String, std::vector<DWORD> Opcodes, int Offset, bool Backwards, bool bRelative, bool bTakeAddressOfOpcode, bool bExactString,  uintptr_t* OutStringAddress, uintptr_t* OutLeaInstructionAddress)
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

			if (bExactString)
			{
				if (std::wstring((const wchar_t*)(StringAddress)) == std::wstring(String))
				{
					if (OutStringAddress)
					{
						*OutStringAddress = StringAddress;
					}
					if (OutLeaInstructionAddress)
					{
						*OutLeaInstructionAddress = reinterpret_cast<unsigned long long>(&Bytes[i]);
					}
					
					Bytes = &Bytes[i];
					bFound = true;
					break;
				}
			}
			else if (wcsstr((const wchar_t*)(StringAddress), String))
			{
				if (OutStringAddress)
				{
					*OutStringAddress = StringAddress;
				}
				if (OutLeaInstructionAddress)
				{
					*OutLeaInstructionAddress = reinterpret_cast<unsigned long long>(&Bytes[i]);
				}
				
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
						if (bRelative)
						{
							return Addr + Offset + 4 + *(__int32*)(Addr + Offset);
						}
						if (bTakeAddressOfOpcode)
						{
							return Addr;
						}

						return *reinterpret_cast<int*>(Addr + Offset);
					}
				}
				else
				{
					break;
				}
			}
		}
	}

	// clever way of finding out where a function starts (only works on Windows x64)
	DWORD64 ImgBase = 0;
	PRUNTIME_FUNCTION Function = nullptr;
	for (auto Func = RtlLookupFunctionEntry(__int64(Bytes), &ImgBase, NULL); Func; Func = RtlLookupFunctionEntry(BaseAddress + (Func->BeginAddress-1), &ImgBase, NULL))
	{
		// make sure the RUNTIME_FUNCTION's unwind info is the primary unwind info of the function
		if (reinterpret_cast<PUNWIND_INFO>(BaseAddress + Func->UnwindInfoAddress)->Flags & UNW_FLAG_CHAININFO) // if it has the CHAININFO flag, not a primary unwind info
		{
			continue; // look into the next function_entry from this point
		}
		
		Function = Func;
		break;
	}

	return Function ? BaseAddress + Function->BeginAddress : 0;
}

uintptr_t memory_manager::get_function_start(uintptr_t address)
{
	uintptr_t BaseAddress = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));
	
	// clever way of finding out where a function starts (only works on Windows x64)
	DWORD64 ImgBase = 0;
	PRUNTIME_FUNCTION Function = nullptr;
	for (auto Func = RtlLookupFunctionEntry(address, &ImgBase, NULL); Func; Func = RtlLookupFunctionEntry(BaseAddress + (Func->BeginAddress-1), &ImgBase, NULL))
	{
		// make sure the RUNTIME_FUNCTION's unwind info is the primary unwind info of the function
		if (reinterpret_cast<PUNWIND_INFO>(BaseAddress + Func->UnwindInfoAddress)->Flags & UNW_FLAG_CHAININFO) // if it has the CHAININFO flag, not a primary unwind info
		{
			continue; // look into the next function_entry from this point
		}
		
		Function = Func;
		break;
	}

	return Function ? BaseAddress + Function->BeginAddress : 0;
}

// Heavily inspired by https://github.com/khalladay/hooking-by-example AllocatePageNearAddress
uintptr_t memory_manager::allocate_page_near_address(uintptr_t address, ENewPageLocation location)
{
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);
	const uintptr_t PAGE_SIZE = sys_info.dwPageSize;

	uintptr_t start_address = address & ~(PAGE_SIZE - 1);
	uintptr_t min_address = std::min(start_address - 0x7FFFFF00, reinterpret_cast<uintptr_t>(sys_info.lpMinimumApplicationAddress));
	uintptr_t max_address = std::max(start_address + 0x7FFFFF00,  reinterpret_cast<uintptr_t>(sys_info.lpMaximumApplicationAddress));

	uintptr_t start_page = start_address - start_address % PAGE_SIZE;

	for (uintptr_t page_offset = 1; page_offset; page_offset++)
	{
		uintptr_t byte_offset = page_offset * PAGE_SIZE;
		uintptr_t high_address = start_page + byte_offset;
		uintptr_t low_address = start_page > byte_offset ? start_page - byte_offset : 0;

		bool bNeedsExit = high_address > max_address && low_address < min_address;

		if (high_address < max_address &&
			location == ENewPageLocation::BOTH ||
			location == ENewPageLocation::HIGH)
		{
			uintptr_t out_address = reinterpret_cast<uintptr_t>(VirtualAlloc(reinterpret_cast<LPVOID>(high_address), PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));

			if (out_address)
			{
				return out_address;
			}
		}

		if (low_address > min_address &&
			location == ENewPageLocation::BOTH ||
			location == ENewPageLocation::LOW)
		{
			uintptr_t out_address = reinterpret_cast<uintptr_t>(VirtualAlloc(reinterpret_cast<LPVOID>(low_address), PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));

			if (out_address)
			{
				return out_address;
			}
		}

		if (bNeedsExit)
		{
			break;
		}
	}

	return 0;
}

uintptr_t memory_manager::get_function_end(uintptr_t function_start)
{
	uintptr_t instruction_address = function_start;

	while (true)
	{
		if (get_function_start(instruction_address) != function_start)
		{
			return instruction_address - 1;
		}
		
		instruction_address++;
	}
}

std::vector<uintptr_t> memory_manager::get_all_calls_in_function(uintptr_t function_start)
{
	std::vector<uintptr_t> calls_list;

	uintptr_t function_end = get_function_end(function_start);
	uintptr_t instruction_address = function_start;

	for (; instruction_address <= function_end; instruction_address++)
	{
		if (*reinterpret_cast<uint8_t*>(instruction_address) == 0xE8) // Call
		{
			uintptr_t called_function = instruction_address + 5 + *reinterpret_cast<int*>(instruction_address + 1);

			if (is_valid_function(called_function))
			{
				calls_list.push_back(instruction_address);
			}
		}
	}

	return calls_list;
}

std::vector<uintptr_t> memory_manager::get_all_leas_in_function(uintptr_t function_start)
{
	std::vector<uintptr_t> calls_list;

	uintptr_t function_end = get_function_end(function_start);
	uintptr_t instruction_address = function_start;

	for (; instruction_address <= function_end; instruction_address++)
	{
		if (*reinterpret_cast<uint8_t*>(instruction_address) == 0x48 && *reinterpret_cast<uint8_t*>(instruction_address + 1) == 0x8D && *reinterpret_cast<uint8_t*>(instruction_address + 2) == 0x05) // Lea
			{
			uintptr_t called_function = instruction_address + 7 + *reinterpret_cast<int*>(instruction_address + 3);

			if (is_valid_function(called_function))
			{
				calls_list.push_back(instruction_address);
			}
			}
	}

	return calls_list;
}

std::vector<uintptr_t> memory_manager::get_all_stripped_calls_in_function(uintptr_t function_start)
{
	std::vector<uintptr_t> stripped_calls_list;
	
	auto all_calls = get_all_calls_in_function(function_start);

	for (auto call : all_calls)
	{
		if (is_stripped_function(call + 5 + *reinterpret_cast<int*>(call + 1)))
		{
			stripped_calls_list.push_back(call);
		}
	}

	return stripped_calls_list;
}

std::vector<uintptr_t> memory_manager::get_all_stripped_calls_in_function_wiktor(uintptr_t function_start)
{
	std::vector<uintptr_t> stripped_calls_list;
	
	auto all_calls = get_all_calls_in_function(function_start);

	for (auto call : all_calls)
	{
		if (is_function_stripped_wiktor(call + 5 + *reinterpret_cast<int*>(call + 1)))
		{
			stripped_calls_list.push_back(call);
		}
	}

	return stripped_calls_list;
}

std::vector<uintptr_t> memory_manager::get_all_stripped_leas_in_function(uintptr_t function_start)
{
	std::vector<uintptr_t> stripped_calls_list;
    
	auto all_calls = get_all_leas_in_function(function_start);

	for (auto call : all_calls)
	{
		if (is_function_stripped_wiktor(call + 7 + *reinterpret_cast<int*>(call + 3)))
		{
			stripped_calls_list.push_back(call);
		}
	}

	return stripped_calls_list;
}

bool memory_manager::is_valid_function(uintptr_t function)
{
	auto BaseAddress = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));
	
	PIMAGE_NT_HEADERS64 Headers = ImageNtHeader(HINSTANCE(BaseAddress));

	//The first section is always .text
	PIMAGE_SECTION_HEADER TextSection = IMAGE_FIRST_SECTION(Headers);

	if (function > BaseAddress + TextSection->VirtualAddress &&
		function < BaseAddress + TextSection->VirtualAddress + TextSection->Misc.VirtualSize)
	{
		return true;
	}

	return false;
}

bool memory_manager::is_stripped_function(uintptr_t function)
{
	return reinterpret_cast<uint8_t*>(function)[0] == 0xe9 ||	// JMP
	       reinterpret_cast<uint8_t*>(function)[0] == 0xeb ||	// JMP
		   reinterpret_cast<uint8_t*>(function)[0] == 0xc3 ||	// RET
		   reinterpret_cast<uint8_t*>(function)[1] == 0xc3 ||	// RET
		   reinterpret_cast<uint8_t*>(function)[2] == 0xc3;		// RET
}

bool memory_manager::is_function_stripped_wiktor(uintptr_t function)
{
	for (int x = 0; x < 8; x++)
	{
		auto Byte = reinterpret_cast<uint8_t*>(function)[x];
		if (Byte == 0xC3 || Byte == 0xC2 || Byte == 0xEB)
			return true;
	}
	return false;
}

uintptr_t memory_manager::find_pe_function(const wchar_t* FunctionString)
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

			if (wcsstr((const wchar_t*)(StringAddress), FunctionString))
			{
				Bytes = &Bytes[i];
				bFound = true;
				break;
			}
		}
	}

	if (!bFound) return 0;

	uintptr_t Function = (uintptr_t)Bytes;
	Function = memory_manager::scan_for_wiktor(Function, "48 8D 0D");
	Function = Function + 7 + *(__int32*)(Function + 3);

	uintptr_t FNameAddress = Function;
	bFound = false;

	Bytes = reinterpret_cast<uint8_t*>(BaseAddress + TextSection->VirtualAddress);
	for (DWORD i = 0x0; i < TextSection->Misc.VirtualSize; i++)
	{
		if ((Bytes[i] == 0x48 && Bytes[i + 1] == 0x8B && Bytes[i + 2] == 0x15))
		{
			void* stringAdd = &Bytes[i];

			stringAdd = (void*)(__int64(stringAdd) + 7 + *(__int32*)(__int64(stringAdd) + 3));

			// Check if the string is in the .rdata section
			if (!memory_manager::is_bad_read_pointer(stringAdd))
			{
				if ((uintptr_t)stringAdd == FNameAddress)
				{
					auto funcAdd = &Bytes[i];
					Function = (uintptr_t)funcAdd;

					DWORD64 ImgBase = 0;
					PRUNTIME_FUNCTION pFunction = nullptr;
					for (auto Func = RtlLookupFunctionEntry(__int64(Function), &ImgBase, NULL); Func; Func = RtlLookupFunctionEntry(BaseAddress + (Func->BeginAddress-1), &ImgBase, NULL))
					{
						pFunction = Func;
					}

					return pFunction ? BaseAddress + pFunction->BeginAddress : 0;
				}
			}
		}
	}

	return 0;
}

uintptr_t memory_manager::scan_for_wiktor(uintptr_t begin, const char* signature, int skip, int scanSize, bool Backwards)
{
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

	std::vector<int> opcodesToFind = patternToByte(signature);
	uint8_t* scanBytes = (uint8_t*)begin;
	int skipped = 0;
	int numfound = 0;

	int ignored = 0;

	bool found = false;

	bool forward = !Backwards;

	void* track = 0;

	for (auto i = 0; forward ? (i < scanSize) : (i > -scanSize); forward ? i++ : i--)
	{
		for (auto op : opcodesToFind)
		{
			if ((op != -1) && scanBytes[i + numfound] != op)
			{
				numfound = 0;
				break;
			}

			track = &scanBytes[i];
			numfound++;
			if (numfound == (opcodesToFind.size()))
			{
				if (skip != skipped)
				{
					skipped++;
					numfound = 0;
					break;
				}

				found = true;
				goto end;
			}
		}
	}

	end:

		if (!found)
			return 0;

	return (uintptr_t)track;
}

void memory_manager::call_hook(uintptr_t call_instruction_address, uintptr_t hook_address)
{
	uintptr_t new_page_address = allocate_page_near_address(call_instruction_address, ENewPageLocation::HIGH);

	// Add the below instructions to the new page
	// mov r10, addr
	// jmp r10

	uint8_t instructions[] = {0x49, 0xBA, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x41, 0xFF, 0xE2};
	
	*reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(instructions) + 2) = hook_address;

	memcpy(reinterpret_cast<LPVOID>(new_page_address), instructions, sizeof instructions);

	// Now, edit the call instruction to call our newly allocated page
	int displacement = new_page_address - (call_instruction_address + 5);
	
	DWORD old_protect, new_protect;
	VirtualProtect(reinterpret_cast<LPVOID>(call_instruction_address), 5, PAGE_EXECUTE_READWRITE, &old_protect);

	*reinterpret_cast<int*>(call_instruction_address + 1) = displacement;
	
	VirtualProtect(reinterpret_cast<LPVOID>(call_instruction_address), 5, old_protect, &new_protect);
}

void memory_manager::lea_hook(uintptr_t lea_instruction_address, uintptr_t hook_address)
{
	uintptr_t new_page_address = allocate_page_near_address(lea_instruction_address, ENewPageLocation::HIGH);

	// Add the below instructions to the new page
	// mov r10, addr
	// jmp r10

	uint8_t instructions[] = {0x49, 0xBA, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x41, 0xFF, 0xE2};
	
	*reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(instructions) + 2) = hook_address;

	memcpy(reinterpret_cast<LPVOID>(new_page_address), instructions, sizeof instructions);

	// Now, edit the lea instruction to work with our newly allocated page
	int displacement = new_page_address - (lea_instruction_address + 7);
	
	DWORD old_protect, new_protect;
	VirtualProtect(reinterpret_cast<LPVOID>(lea_instruction_address), 7, PAGE_EXECUTE_READWRITE, &old_protect);

	*reinterpret_cast<int*>(lea_instruction_address + 3) = displacement;
	
	VirtualProtect(reinterpret_cast<LPVOID>(lea_instruction_address), 7, old_protect, &new_protect);
}

uintptr_t memory_manager::scan_for(uintptr_t start_address, std::vector<DWORD> Opcodes, uintptr_t optional_end_address, bool Backwards)
{
	uintptr_t end_address = optional_end_address ? optional_end_address : Backwards ? start_address - 2048 : start_address + 2048;

	for (uintptr_t current_address = start_address; Backwards ? current_address > end_address : current_address < end_address; Backwards ? current_address-- : current_address++)
	{
		uint8_t* Byte = reinterpret_cast<uint8_t*>(current_address);

		for (auto i = 0; i < Opcodes.size(); i++)
		{
			if (Byte[i] == Opcodes[i] || Opcodes[i] == 0x100)
			{
				if (i+1 == Opcodes.size())
				{
					return current_address;
				}
			}
			else
			{
				break;
			}
		}
	}
	
	return 0;
}

bool memory_manager::does_address_belong_to_func(uintptr_t target_address, uintptr_t function_start)
{
	return target_address < get_function_end(function_start) && target_address > function_start;
}

bool memory_manager::is_bad_read_pointer(void* pointer_to_read, bool use_cache)
{
	if (use_cache)
	{
		if (pointer_cache.find(pointer_to_read) != pointer_cache.end())
		{
			return pointer_cache[pointer_to_read];
		}
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

