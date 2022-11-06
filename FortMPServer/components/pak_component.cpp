#include "include.hpp"
#include "pak_component.hpp"

pak_component::pak_component()
{
    INIT_POINTER(FPlatformFileManager__Get, memory_manager::find_pattern("E8 ? ? ? ? 48 8B C8 48 8D 15 ? ? ? ? E8 ? ? ? ? 48 85 C0", true, 1));

    INIT_POINTER(FPlatformFileManager__FindPlatformFile, memory_manager::find_pattern("48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 83 EC 20 48 8B 31"));
    INIT_POINTER(FPlatformFileManager__FindPlatformFile, memory_manager::find_pattern("48 89 5C 24 ? 57 48 83 EC 20 48 8B 19 48 8B FA 48 85 DB 74 29"));

    INIT_POINTER(FPakPlatformFile__Mount, memory_manager::find_function_by_string(L"Failed to mount pak \"%s\", pak is invalid."));

    // Get Current Directory
    auto ModsPath = std::string(dll_utils::get_current_path()) + "/Mods";

    // Create directory if it doesn't exist
    std::filesystem::create_directories(ModsPath);
    
    // Delete sig files
    for (const auto& Entry : std::filesystem::directory_iterator(ModsPath))
    {
        if (Entry.path().string().find(".sig") != std::string::npos)
        {
            std::remove(Entry.path().string().c_str());
        }
    }

    // Add a .sig file for each pak
    for (const auto& Entry : std::filesystem::directory_iterator(ModsPath))
    {
        std::string file_name_full = Entry.path().string();

        // If it's a pak file, proceed
        if (file_name_full.find(".pak") != std::string::npos)
        {
            std::string pak_file_name = file_name_full.substr(file_name_full.find_last_of("/\\") + 1);
            pak_file_name = pak_file_name.substr(0, pak_file_name.find(".pak"));

            std::string sig_file_name = pak_file_name.append(".sig");

            // Take the first .sig file from the Paks folder
            CHAR CurrentPath[MAX_PATH];

            GetCurrentDirectoryA(MAX_PATH, CurrentPath);

            std::string strCurrentPath = std::string(CurrentPath);

            strCurrentPath = strCurrentPath.substr(0, strCurrentPath.find("Binaries"));

            std::string PaksPath = strCurrentPath.append("\\Content\\Paks");

            for (const auto& Entry_ : std::filesystem::directory_iterator(PaksPath))
            {
                if (Entry_.path().string().find(".sig") != std::string::npos)
                {
                    std::filesystem::copy(Entry_.path().string(), ModsPath + "/" + sig_file_name);

                    spdlog::info("Added .sig file for Custom Pak. New sig file: '{}'", pak_file_name.c_str());
                        
                    break;
                }
            }
        }
    }
    
    auto FModsPath = FString(std::wstring(ModsPath.begin(), ModsPath.end()).c_str());

    // Setup PakFolders parameter
    TArray<FString> PakFolders;
    PakFolders.Add(FModsPath);

    // Get platform file manager
    void* PlatformFileManager = FPlatformFileManager__Get();

    // Get pak file
    void* PakPlatformFile = FPlatformFileManager__FindPlatformFile(PlatformFileManager, L"PakFile");

    // Mount pak
    int NumPaksLoaded = 0;
    
    std::vector<std::wstring> FoundPakFiles;

    FindAllPakFiles(FoundPakFiles);

    for (std::wstring PakFile : FoundPakFiles)
    {
        if (FPakPlatformFile__Mount(PakPlatformFile, PakFile.c_str(), 0))
        {
            NumPaksLoaded++;
        }
    }

    spdlog::info(L"Loaded {} Server Paks", NumPaksLoaded);
}

void pak_component::FindAllPakFiles(std::vector<std::wstring>& FoundPakFiles)
{
    // Get Current Directory
    auto ModsPath = std::string(dll_utils::get_current_path()) + "/Mods";

    // Create directory if it doesn't exist
    std::filesystem::create_directories(ModsPath);

    for (const auto& Entry : std::filesystem::directory_iterator(ModsPath))
    {
        std::string file_name_full = Entry.path().string();

        // If it's a pak file, proceed
        if (file_name_full.find(".pak") != std::string::npos)
        {
            std::wstring PakFilename = std::wstring(file_name_full.begin(), file_name_full.end()).c_str();

            FoundPakFiles.push_back(PakFilename);
        }
    }
}
