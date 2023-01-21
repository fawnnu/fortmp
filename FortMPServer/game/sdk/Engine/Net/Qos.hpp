#pragma once

enum class EIcmpResponseStatus
{
    /** We did receive a valid Echo reply back from the target host */
    Success,
    /** We did not receive any results within the time limit */
    Timeout,
    /** We got an unreachable error from another node on the way */
    Unreachable,
    /** We could not resolve the target address to a valid IP address */
    Unresolvable,
    /** Some internal error happened during setting up or sending the ping packet */
    InternalError,
    /** not implemented - used to indicate we haven't implemented ICMP ping on this platform */
    NotImplemented,

};

struct FIcmpEchoResult
{
    FIcmpEchoResult() = default;
    
    /** Status of the final response */
    EIcmpResponseStatus Status;
    /** Addressed resolved by GetHostName */
    FString ResolvedAddress;
    /** Reply received from this address */
    FString ReplyFrom;
    /** Total round trip time */
    float Time;
};

struct FQosPingServerInfo
{
    DEFINE_STRUCT_PROPERTY(FString, Address);
    DEFINE_STRUCT_PROPERTY(int, Port);        
   
    DEFINE_STRUCT_CLASS(L"/Script/Qos.QosPingServerInfo")
};

struct FQosDatacenterInfo
{
    DEFINE_STRUCT_PROPERTY(FString, ID);                                
    DEFINE_STRUCT_PROPERTY(FString, RegionId);                          
    DEFINE_STRUCT_PROPERTY(bool, bEnabled);                          
    DEFINE_STRUCT_PROPERTY(TArray<FQosPingServerInfo>, Servers);

    DEFINE_STRUCT_CLASS(L"/Script/Qos.QosDatacenterInfo")
};

class UQosRegionManager : public UObject
{
public:
    DEFINE_STATIC_CLASS(L"/Script/Qos.QosRegionManager");

    DEFINE_PROPERTY(int, NumTestsPerRegion);
    DEFINE_PROPERTY(float, PingTimeout);
    
    TArray<FQosDatacenterInfo>& DatacenterDefinitions()
    {
        static void* native_property = FindPropertyByName(this->Class, "DatacenterDefinitions") ? FindPropertyByName(this->Class, "DatacenterDefinitions") : FindPropertyByName(this->Class, "Datacenters");
        static int native_property_offset = *reinterpret_cast<int*>(__int64(native_property) + CoreOffsets::Offset_Internal);

        return *reinterpret_cast<TArray<FQosDatacenterInfo>*>(__int64(this) + native_property_offset);
    };
};