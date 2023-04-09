#pragma once

class UNetDriver;
class UChildConnection;

class UPlayer : public UObject
{
public:
    DEFINE_PROPERTY(class AFortPlayerController*, PlayerController);
};

class UPackageMap : public UObject
{
public:
};

enum EConnectionState
{
    USOCK_Invalid   = 0, // Connection is invalid, possibly uninitialized.
    USOCK_Closed    = 1, // Connection permanently closed.
    USOCK_Pending	= 2, // Connection is awaiting connection.
    USOCK_Open      = 3, // Connection is open.
};

class UNetConnection : public UPlayer
{
public:
    DEFINE_STATIC_CLASS(L"/Script/Engine.NetConnection")
    
    DEFINE_PROPERTY(TArray<UChildConnection*>, Children);
    
    DEFINE_PROPERTY(AActor*, ViewTarget);
    DEFINE_PROPERTY(AActor*, OwningActor);

    DEFINE_PROPERTY(UNetDriver*, Driver);
    
    DEFINE_PROPERTY(FUniqueNetIdRepl, PlayerID);

    DEFINE_PROPERTY(uint8_t, InternalAck);

    DEFINE_PROPERTY(double, LastReceiveTime);

    DEFINE_PROPERTY(TArray<UChannel*>, OpenChannels);

    DEFINE_PROPERTY(TArray<AActor*>, SentTemporaries);
    
    DEFINE_PROPERTY(UPackageMap*, PackageMap);

    EConnectionState State()
    {
        static void* native_property = FindPropertyByName(this->Class, "PlayerId");                                                                              
        static int native_property_offset = *reinterpret_cast<int*>(__int64(native_property) + CoreOffsets::Offset_Internal);

        return *reinterpret_cast<EConnectionState*>(reinterpret_cast<uintptr_t>(this) + native_property_offset - 0x2C);
    }

    void Close()
    {
        NetConnection__Close(this);
    }
};

class UChildConnection : public UNetConnection
{
public:
    DEFINE_PROPERTY(UNetConnection*, Parent);
};

class FInternetAddr
{
public:
    void* VTable;
};

class FInternetAddrBSD : public FInternetAddr
{
public:
    sockaddr_in Addr;

    std::string ToString()
    {
        uint32 LocalAddr = ntohl(Addr.sin_addr.s_addr);

        // Get the individual bytes
        const int32 A = (LocalAddr >> 24) & 0xFF;
        const int32 B = (LocalAddr >> 16) & 0xFF;
        const int32 C = (LocalAddr >>  8) & 0xFF;
        const int32 D = (LocalAddr >>  0) & 0xFF;

        char ip_string[14];

        sprintf_s(ip_string, "%i.%i.%i.%i", A, B, C, D);
        
        return ip_string;
    }
};

class UIpConnection : public UNetConnection
{
public:
    TSharedPtr<FInternetAddrBSD>& RemoteAddr()
    {
        static int remoteaddr_offset = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(StaticClass()) + CoreOffsets::StructSize);

        return *reinterpret_cast<TSharedPtr<FInternetAddrBSD>*>(reinterpret_cast<uintptr_t>(this) + remoteaddr_offset);
    }
};
