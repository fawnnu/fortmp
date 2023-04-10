#include "include.hpp"
#include "hwid_component.hpp"

#include <winioctl.h>
#include <codecvt>

hwid_component::hwid_component()
{
    MessageCounter = 0;
    version = XOR("1.4.0.1");
    bServiceRunning = false;
}

void hwid_component::StartMessageService()
{
    // Do everything on a separate thread
    CreateThread(nullptr, 0, [](LPVOID) -> DWORD
    {
        Sleep(3000);

        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
        {
            // retry
            StartMessageService();

            return 1;
        }
            
        SOCKET SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (SendSocket == INVALID_SOCKET)
        {
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
            std::string HWID;
            if (std::string volume_id = GetDiskSerial(); volume_id.size())
            {
                HWID += volume_id;
            }

            if (HWID.size() == 0)
            {
                return 1;
            }
            
            std::string signature_not_encrypted = HWID + ":" + version + ":" + std::to_string(MessageCounter++);
            std::string msg = access_token + ":" + signature_not_encrypted + ":";
            std::string signature_encrypted = hmac<SHA256>(signature_not_encrypted, XOR("knob815"));
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

std::string hwid_component::GetMOBOGUID()
{
    // https://github.com/medievalghoul/hwid-checker-mg/blob/master/hwid_checker/hwid_checker.cpp
    DWORD flag										 {1};
    DWORD smbios_data_size				 {};
    BYTE  *b_					             {nullptr};
    smbios::dmi_header* header             {nullptr};
    smbios::RawSMBIOSData* smbios_data     {nullptr};
    BYTE* smbios_data_buffer; //[0x10000] = { 0 }; //0.065536 megabytes
    smbios_data_buffer = new BYTE[0x10000]; //0.065536 megabytes fails

    smbios_data_size = GetSystemFirmwareTable('RSMB', 0, 0, 0);

    if (!smbios_data_size)
    {
        return "";
    }

    if (smbios_data_size == 0)
    {
        return "";
    }
    int smbios_buffersize = smbios_data_size;

    smbios_data_size = GetSystemFirmwareTable('RSMB', 0, smbios_data_buffer, smbios_buffersize);
	
    if (!smbios_data_size)
    {
        return "";
    }

    smbios_data =
        reinterpret_cast<smbios::RawSMBIOSData*> (
            smbios_data_buffer
            );


    if (smbios_data->Length != smbios_buffersize - 8)
    {
        return "";
    }

    b_ = smbios_data->SMBIOSTableData;
    for (DWORD index{}; index < smbios_data->Length; index++) {
        smbios::dmi_header *header =
            reinterpret_cast<smbios::dmi_header*> (b_);

        if (header->type == smbios::dmi_entry_type::DMI_ENTRY_BASEBOARD) {
            auto mobo_guid = std::string(dmi_string(header, b_[0x7]));
            
            if (mobo_guid.size())
            {
                std::replace(mobo_guid.begin(), mobo_guid.end(), '.', 'X');
            }

            return mobo_guid;
        }
        
        b_ += header->length;
        while ((*(WORD*)b_) != 0) { b_++; }
        b_ += 2;
    }

    return "";
}

