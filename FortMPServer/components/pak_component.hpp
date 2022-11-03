#pragma once

class pak_component
{
public:
    pak_component();
};

inline void* (*FPlatformFileManager__Get)();

inline void* (*FPlatformFileManager__FindPlatformFile)(void* Self, const TCHAR* Name);

inline int (*FPakPlatformFile__MountAllPakFiles)(void* Self, TArray<FString>& PakFolders);
inline int (*FPakPlatformFile__MountAllPakFiles_New)(void* Self, TArray<FString>& PakFolders, const FString& Wildcard);
