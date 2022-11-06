#pragma once

class pak_component
{
public:
    pak_component();

    static void FindAllPakFiles(std::vector<std::wstring>& FoundPakFiles);
};

inline void* (*FPlatformFileManager__Get)();

inline void* (*FPlatformFileManager__FindPlatformFile)(void* Self, const TCHAR* Name);

inline bool (*FPakPlatformFile__Mount)(void* Self, const TCHAR* InPakFilename, uint32_t PakOrder);
