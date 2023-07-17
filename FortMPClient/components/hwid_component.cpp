#include "include.hpp"
#include "hwid_component.hpp"

#include <winioctl.h>
#include <codecvt>
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

hwid_component::hwid_component()
{
    MessageCounter = 0;
    version = XOR("1.6.2");
    bServiceRunning = false;
    bPendingShutdown = false;
}

void hwid_component::StartMessageService()
{
    // Do everything on a separate thread
    CreateThread(nullptr, 0, [](LPVOID) -> DWORD
    {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
        {
            Sleep(3000);

            // retry
            StartMessageService();

            return 1;
        }
            
        SOCKET SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (SendSocket == INVALID_SOCKET)
        {
            Sleep(3000);

            WSACleanup();
            
            // retry
            StartMessageService();

            return 1;
        }
        
        sockaddr_in target_addr;
        target_addr.sin_family = AF_INET;
        target_addr.sin_port = htons(2052);
        target_addr.sin_addr.s_addr = inet_addr(XOR("51.81.67.165"));
    
        while (true)
        {
            if (bPendingShutdown)
            {
                return 1;
            }
            
            std::string HWID = GetHashedHWID();
            if (HWID.size() == 0)
            {
                MessageBoxA(0, XOR("An error occurred while attempting to log in. Please report this error."), 0, 0);

                return 1;
            }
            
            std::string signature_not_encrypted = HWID + ":" + version + ":" + std::to_string(MessageCounter++);
            std::string msg = access_token + ":" + signature_not_encrypted + ":";
            std::string signature_encrypted = hmac<sha256lib::SHA256>(signature_not_encrypted, XOR("knob815"));
            std::transform(signature_encrypted.begin(), signature_encrypted.end(), signature_encrypted.begin(), toupper);
            msg += signature_encrypted;
            msg = base64::to_base64(msg);

            // Trim trailing =
            while (msg.back() == '=')
            {
                msg.pop_back();
            }
            std::replace(msg.begin(), msg.end(), '+', '-');
            std::replace(msg.begin(), msg.end(), '/', '_');

            msg = ":" + msg;
            
            if (sendto(SendSocket, msg.data(), msg.size(), 0, reinterpret_cast<SOCKADDR*>(&target_addr), sizeof target_addr) == SOCKET_ERROR)
            {
                closesocket(SendSocket);
                WSACleanup();
            
                // retry
                StartMessageService();
            
                return 1;
            }

            Sleep(10000);
        }
        
        return 0;
    }, nullptr, 0, nullptr);
}

std::string hwid_component::GetDiskSerial()
{
    // SRC: GuidedHacking
    std::string m_sResult;
 
    HANDLE m_hFile = CreateFileW(XOR(L"\\\\.\\PhysicalDrive0"), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (m_hFile == INVALID_HANDLE_VALUE)
    {
        return "";
    }
    
    std::unique_ptr< std::remove_pointer <HANDLE >::type, void(*)(HANDLE) > m_hDevice
    {
        m_hFile, [](HANDLE handle)
        {
            CloseHandle(handle);
        }
    };
 
    STORAGE_PROPERTY_QUERY m_PropertyQuery;
    m_PropertyQuery.PropertyId = StorageDeviceProperty;
    m_PropertyQuery.QueryType = PropertyStandardQuery;
 
    STORAGE_DESCRIPTOR_HEADER m_DescHeader;
    DWORD m_dwBytesReturned = 0;
    if (!DeviceIoControl(m_hDevice.get(), IOCTL_STORAGE_QUERY_PROPERTY, &m_PropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
        &m_DescHeader, sizeof(STORAGE_DESCRIPTOR_HEADER), &m_dwBytesReturned, NULL))
    { }
 
    const DWORD m_dwOutBufferSize = m_DescHeader.Size;
    std::unique_ptr< BYTE[] > m_pOutBuffer{ new BYTE[m_dwOutBufferSize] { } };
 
    if (!DeviceIoControl(m_hDevice.get(), IOCTL_STORAGE_QUERY_PROPERTY, &m_PropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
        m_pOutBuffer.get(), m_dwOutBufferSize, &m_dwBytesReturned, NULL))
    { }
 
    STORAGE_DEVICE_DESCRIPTOR* m_pDeviceDescriptor = reinterpret_cast<STORAGE_DEVICE_DESCRIPTOR*>(m_pOutBuffer.get());
    const DWORD m_dwSerialNumberOffset = m_pDeviceDescriptor->SerialNumberOffset;
    if (m_dwSerialNumberOffset == 0)
    { }
 
    m_sResult = reinterpret_cast<const char*>(m_pOutBuffer.get() + m_dwSerialNumberOffset);

    if (m_sResult.size())
    {
        std::replace(m_sResult.begin(), m_sResult.end(), '.', 'X');
        m_sResult = base64::to_base64(m_sResult);

        // Trim trailing =
        while (m_sResult.back() == '=')
        {
            m_sResult.pop_back();
        }
        std::replace(m_sResult.begin(), m_sResult.end(), '+', '-');
        std::replace(m_sResult.begin(), m_sResult.end(), '/', '_');
        std::replace(m_sResult.begin(), m_sResult.end(), '.', 'X');
    }

    return m_sResult;
}

std::string hwid_component::GetGPUGUID()
{
    // https://github.com/medievalghoul/hwid-checker-mg/blob/master/hwid_checker/hwid_checker.cpp
    IDirect3D9 *d9object = Direct3DCreate9(D3D_SDK_VERSION);
    if (d9object)
    {
        unsigned int adaptercount = d9object->GetAdapterCount();

        if (adaptercount)
        {
            D3DADAPTER_IDENTIFIER9* adapters = new D3DADAPTER_IDENTIFIER9[sizeof(adaptercount)];

            for (unsigned int i = 0; i < adaptercount; i++) {
                d9object->GetAdapterIdentifier(i, 0, &(adapters[i]));
            }

            if (adapters)
            {
                auto gpu_guid = adapters->DeviceIdentifier;

                WCHAR* gpu_guid_wstring;
                StringFromCLSID(gpu_guid, &gpu_guid_wstring);

                std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
                std::string gpu_guid_string = converter.to_bytes(gpu_guid_wstring);

                if (gpu_guid_string.size())
                {
                    std::replace(gpu_guid_string.begin(), gpu_guid_string.end(), '.', 'X');
                }

                return gpu_guid_string;
            }
        }
    }

    return "";
}

std::string hwid_component::GetSysUUID()
{
    WCHAR* sys_uuid_wstring;
    StringFromCLSID(SMBIOS::getInstance().SysUUID(), &sys_uuid_wstring);
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    std::string sys_uuid_string = converter.to_bytes(sys_uuid_wstring);
    
    return sys_uuid_string;
}

std::string hwid_component::GetBoardSerial()
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    std::string string_board_serial = converter.to_bytes(SMBIOS::getInstance().BoardSerialNumber());
    
    return string_board_serial;
}

std::string hwid_component::GetBootUUID()
{
    std::string boot_uuid;
    NTSTATUS (*ZwQuerySystemInformation)(int SystemInformationClass,PVOID SystemInformation,ULONG SystemInformationLength, PULONG ReturnLength);
    
    if (HINSTANCE NtDllInstance = LoadLibraryA(XOR("ntdll.dll")))
    {
        if (auto ZwQuerySystemInformation_Address = GetProcAddress(NtDllInstance, XOR("ZwQuerySystemInformation")))
        {
            ZwQuerySystemInformation = reinterpret_cast<decltype(ZwQuerySystemInformation)>(ZwQuerySystemInformation_Address);

            ULONG NeededSize = 0;
            ZwQuerySystemInformation(0x5A, &NeededSize, 0, &NeededSize);

            // Alloc
            auto ModuleList = static_cast<PSYSTEM_BOOT_ENVIRONMENT_INFORMATION>(malloc(NeededSize));

            if (ModuleList)
            {
                if (NT_SUCCESS(ZwQuerySystemInformation(0x5A, ModuleList, NeededSize, 0)))
                {
                    WCHAR* boot_uuid_wstring;
                    StringFromCLSID(ModuleList->BootIdentifier, &boot_uuid_wstring);
                    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
                    std::string boot_uuid_string = converter.to_bytes(boot_uuid_wstring);
                    
                    return boot_uuid_string;
                }
            }
        }
    }

    return boot_uuid;
}

std::string hwid_component::GetHashedHWID()
{
    std::string HWID;
    bool bSuccessfulRegistryKeyRead = false;
    
    HKEY Key;
    LSTATUS Status = RegOpenKeyExA(HKEY_CURRENT_USER, XOR("SOFTWARE\\Microsoft\\UserCache"), 0, KEY_READ, &Key);
    if (Status == ERROR_SUCCESS)
    {
        char Value[255];
        DWORD BufferSize = 8192;
        if (RegGetValueA(HKEY_CURRENT_USER, XOR("SOFTWARE\\Microsoft\\UserCache"), XOR("CachedHash"), RRF_RT_ANY, NULL, &Value, &BufferSize) == ERROR_SUCCESS)
        {
            if (const std::string sValue = std::string(Value); sValue.length() > 0)
            {
                HWID = sValue;
                bSuccessfulRegistryKeyRead = true;
            }
        }
        RegCloseKey(Key);
    }

    if (bSuccessfulRegistryKeyRead == false)
    {
        if (std::string DiskSerial = GetDiskSerial(); DiskSerial.size())
        {
            HWID += DiskSerial;
        }
        if (std::string SysUUID = GetSysUUID(); SysUUID.size())
        {
            HWID += SysUUID;
        }
        if (std::string BootUUID = GetBootUUID(); BootUUID.size())
        {
            HWID += BootUUID;
        }
        if (std::string BoardSerial = GetBoardSerial(); BoardSerial.size())
        {
            HWID += BoardSerial;
        }

        if (HWID.size() == 0)
        { 
            return XOR("");
        }

        // Hash HWID
        HWID = hmac<sha256lib::SHA256>(HWID, XOR("1681819033_DANII"));

        // Create the key and set the value
        HKEY NewKey;
        if (RegCreateKeyA(HKEY_CURRENT_USER, XOR("SOFTWARE\\Microsoft\\UserCache"), &NewKey) == ERROR_SUCCESS)
        {
            if (RegSetKeyValueA(HKEY_CURRENT_USER, XOR("SOFTWARE\\Microsoft\\UserCache"), XOR("CachedHash"), REG_EXPAND_SZ, HWID.c_str(), HWID.size() + 1) ==  ERROR_SUCCESS)
            {
            }
        }
    }
    
    return HWID;
}


