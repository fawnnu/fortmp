#include "include.hpp"
#include "hwid_component.hpp"

hwid_component::hwid_component()
{
    MessageCounter = 0;
    version = XOR("1.4.0");
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
            //TODO: Get real HWID
            std::string signature_not_encrypted = std::string("HWID") + ":" + version + ":" + std::to_string(MessageCounter++);
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

