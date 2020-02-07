#include "net_filter.h"

#include <fwpmk.h>

#pragma warning(push)
#pragma warning(disable:4201)       // unnamed struct/union
#include <fwpsk.h>
#pragma warning(pop)

#include <initguid.h>
#include <ntstrsafe.h>

#include "net_helper.h"

//
// Capture inbound and outbound traffic (cnt=2)
//
// 1) Open filter enfine handle
// 2) Create sublayer
// 3) Create 2 callouts (callback functions for inbound and outbound traffic)
// 4) Create filters for each callout and bind them to callouts
// 5) Setup filter to perform packet inspection at created sublayer
//

#define FLT_CNT 2
#define IDX_INBOUND 0
#define IDX_OUTBOUND 1

// bb6e405b-19f4-4ff3-b501-1a3dc01aae01
DEFINE_GUID(
    NETFILTER_CALLOUT_INBOUND_TRANSPORT,
    0xbb6e405b,
    0x19f4,
    0x4ff3,
    0xb5, 0x01, 0x1a, 0x3d, 0xc0, 0x1a, 0xae, 0x01
);

// 7ec7f7f5-0c55-4121-adc5-5d07d2ac0cef
DEFINE_GUID(
    NETFILTER_CALLOUT_OUTBOUND_TRANSPORT,
    0x7ec7f7f5,
    0x0c55,
    0x4121,
    0xad, 0xc5, 0x5d, 0x07, 0xd2, 0xac, 0x0c, 0xef
);

// 2e207682-d95f-4525-b966-969f26587f03
DEFINE_GUID(
    NETFILTER_SUBLAYER,
    0x2e207682,
    0xd95f,
    0x4525,
    0xb9, 0x66, 0x96, 0x9f, 0x26, 0x58, 0x7f, 0x03
);


PDEVICE_OBJECT gDeviceObject = NULL;
HANDLE gEngineHandle = NULL;

struct CalloutContext
{
	UINT32 regCalloutId = 0;
	UINT32 addCalloutId = 0;
	UINT64 filterId = 0;
};

struct CalloutDescription
{
    wchar_t* name = nullptr;
    wchar_t* description = nullptr;
};

CalloutContext gCalloutCtx[FLT_CNT];

class NetPacketBuffer
{
    PCHAR mContiguousData = nullptr;
    PCHAR mBuffer = nullptr;
    ULONG mLength = 0;

public:

    NetPacketBuffer(PNET_BUFFER pNetBuffer, ULONG bytesNeeded)
    {
        if (bytesNeeded == 0)
            return;

        mBuffer = new CHAR[bytesNeeded];
        if (mBuffer != nullptr)
        {
            mContiguousData = (PCHAR)NdisGetDataBuffer(pNetBuffer,
                bytesNeeded, mBuffer, 1, 0);

            mLength = bytesNeeded;
            if (mBuffer != mContiguousData)
            {
                // Data is contiguous, no allocation needed
                // Or API call failed and buffer no longer required
                delete[] mBuffer;
                mBuffer = nullptr;

                if (mContiguousData == nullptr)
                {
                    mLength = 0;
                    kprintf(TRACE_NETFILTER_ERROR, "NdisGetDataBuffer failed");
                }
            }
        }
        else
			kprintf(TRACE_NETFILTER_ERROR, "Memory allocation failed");
    }

    ~NetPacketBuffer()
    {
        mLength = 0;

        if (mBuffer)
        {
            delete[] mBuffer;
            mBuffer = nullptr;
        }
    }

	// forbid copy/move
    // No need to implement it now
	NetPacketBuffer(const NetPacketBuffer&) = delete;
	NetPacketBuffer(NetPacketBuffer&&) = delete;


    PCHAR getData()
    {
        return mBuffer != nullptr
            ? mBuffer
            : mContiguousData;
    }

    ULONG getLength() {
        return mLength;
    }
};

GuardedMutex gNetFltLock;
PNetFilterRuleList gNetFlt = nullptr;

PNetFilterRuleList NetFilterGetInstancePtr() 
{
	LockGuard<GuardedMutex> guard(&gNetFltLock);
	return gNetFlt;
}

bool NetFilterNewInstance()
{
	LockGuard<GuardedMutex> guard(&gNetFltLock);

	gNetFlt = new NetFilterRuleList(KNF_BASE_KEY);
	gNetFlt->regReadConfiguration();
	return gNetFlt != nullptr;
}

void NetFilterDeleteInstance()
{
	gNetFltLock.acquire();
	delete gNetFlt;
	gNetFlt = nullptr;
	gNetFltLock.release();
}

NTSTATUS NotifyCallback(
   _In_  FWPS_CALLOUT_NOTIFY_TYPE notifyType,
   _In_ const GUID* filterKey,
   _Inout_ FWPS_FILTER* filter
   )
{
   UNREFERENCED_PARAMETER(notifyType);
   UNREFERENCED_PARAMETER(filterKey);
   UNREFERENCED_PARAMETER(filter);

   return STATUS_SUCCESS;
}


#define NETFILTER_ERROR_RETURN(fmt, ...)     \
    return (kprintf(TRACE_NETFILTER_ERROR, fmt, __VA_ARGS__)  \
        ? STATUS_UNSUCCESSFUL : STATUS_UNSUCCESSFUL)

#if(NTDDI_VERSION >= NTDDI_WIN7)

NTSTATUS
FilterCallbackImpl(
   _In_ const FWPS_INCOMING_VALUES* pClassifyValues,
   _In_ const FWPS_INCOMING_METADATA_VALUES* pMetadata,
   _Inout_opt_ void* pLayerData
   )

#else

NTSTATUS
FilterCallbackImpl(
    _In_ const FWPS_INCOMING_VALUES* pClassifyValues,
    _In_ const FWPS_INCOMING_METADATA_VALUES* pMetaData,
    _Inout_opt_ void* pLayerData
)

#endif /// (NTDDI_VERSION >= NTDDI_WIN7)

{
    UNREFERENCED_PARAMETER(pMetadata);
    NT_ASSERT(pLayerData);

    PNET_BUFFER pNetBuffer = NET_BUFFER_LIST_FIRST_NB(
        (NET_BUFFER_LIST*)pLayerData);

    NTSTATUS status = STATUS_SUCCESS;
    FWP_VALUE* pSrcAddrValue = nullptr;
    FWP_VALUE* pDstAddrValue = nullptr;
    FWP_VALUE* pSrcPortValue = nullptr;
    FWP_VALUE* pDstPortValue = nullptr;
    FWP_VALUE* pProtocolValue = nullptr;
    UINT8 protocol = IPPROTO_RAW;

    if (pClassifyValues->layerId == FWPS_LAYER_INBOUND_TRANSPORT_V4)
    {
        pProtocolValue = &(pClassifyValues->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_PROTOCOL].value);
        pDstAddrValue = &(pClassifyValues->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_LOCAL_ADDRESS].value);
        pSrcAddrValue = &(pClassifyValues->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_REMOTE_ADDRESS].value);
        pDstPortValue = &(pClassifyValues->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_LOCAL_PORT].value);
        pSrcPortValue = &(pClassifyValues->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_REMOTE_PORT].value);
    }
    else
    {
        pProtocolValue = &(pClassifyValues->incomingValue[FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_PROTOCOL].value);
        pDstAddrValue = &(pClassifyValues->incomingValue[FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_REMOTE_ADDRESS].value);
        pSrcAddrValue = &(pClassifyValues->incomingValue[FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_LOCAL_ADDRESS].value);
        pDstPortValue = &(pClassifyValues->incomingValue[FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_REMOTE_PORT].value);
        pSrcPortValue = &(pClassifyValues->incomingValue[FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_LOCAL_PORT].value);
    }

    if (!pProtocolValue)
        NETFILTER_ERROR_RETURN("pProtocolValue is NULL");
    else
        protocol = pProtocolValue->uint8;

    if (!pSrcAddrValue)
        NETFILTER_ERROR_RETURN("pSrcAddrValue is NULL");

    if (!pDstAddrValue)
        NETFILTER_ERROR_RETURN("pDstAddrValue is NULL");

    if (!pSrcPortValue)
        NETFILTER_ERROR_RETURN("pSrcPortValue is NULL");

    if (!pDstPortValue)
        NETFILTER_ERROR_RETURN("pDstPortValue is NULL");

    bool allowed;
    const wchar_t* rule = nullptr;
    ULONG DataLength = NET_BUFFER_DATA_LENGTH(pNetBuffer);
    NetPacketBuffer buffer(pNetBuffer, DataLength);

    if (gNetFlt != nullptr)
    {
        rule = gNetFlt->findMatchingRule(
            pProtocolValue->uint8,
            pSrcAddrValue->uint32,
            pSrcPortValue->uint16,
            pDstAddrValue->uint32,
            pDstPortValue->uint16,
            buffer.getData(),
            buffer.getLength(),
            &allowed);

        if (rule != nullptr)
        {
            if (DataLength > 30)
                DataLength = 30;

            HexDeserializer deser(buffer.getData(), DataLength);
            unsigned ip_from = pSrcAddrValue->uint32;
            unsigned ip_to = pDstAddrValue->uint32;

            const char* prot_str = protocol == ICMPV4 
                ? "ICMP" : protocol == TCP 
                ? "TCP" : protocol == UDP 
                ? "UDP" : "Unknown";

            const char* perm_str = allowed ? "Allow" : "Deny";

            if (!allowed)
                status = STATUS_ACCESS_DENIED;

            kprintf(TRACE_NETFILTER_INFO, "%s packet <rule=%ws prot=%s %d.%d.%d.%d:%d -> %d.%d.%d.%d:%d first bytes=%s>",
                perm_str, rule, prot_str, (ip_from & 0xFF000000) >> 24, (ip_from & 0x00FF0000) >> 16, (ip_from & 0x0000FF00) >> 8, ip_from & 0x000000FF,
                pSrcPortValue->uint16, (ip_to & 0xFF000000) >> 24, (ip_to & 0x00FF0000) >> 16, (ip_to & 0x0000FF00) >> 8, ip_to & 0x000000FF,
                pDstPortValue->uint16, deser.getHexText());
        }
    }

    return status;
}

#if(NTDDI_VERSION >= NTDDI_WIN7)

void
FilterCallback(
   _In_ const FWPS_INCOMING_VALUES* pClassifyValues,
   _In_ const FWPS_INCOMING_METADATA_VALUES* pMetadata,
   _Inout_opt_ void* pLayerData,
   _In_opt_ const void* classifyContext,
   _In_ const FWPS_FILTER* filter,
   _In_ UINT64 flowContext,
   _Inout_ FWPS_CLASSIFY_OUT* classifyOut
   )

#else

void
FilterCallbackImpl(
    _In_ const FWPS_INCOMING_VALUES* pClassifyValues,
    _In_ const FWPS_INCOMING_METADATA_VALUES* pMetaData,
    _Inout_opt_ void* pLayerData,
    _In_ const FWPS_FILTER* filter,
    _In_ UINT64 flowContext,
    _Inout_ FWPS_CLASSIFY_OUT* classifyOut
)

#endif /// (NTDDI_VERSION >= NTDDI_WIN7)

{
    NTSTATUS Status = STATUS_SUCCESS;
    UNREFERENCED_PARAMETER(flowContext);
    UNREFERENCED_PARAMETER(classifyContext);
    UNREFERENCED_PARAMETER(filter);

    classifyOut->actionType = FWP_ACTION_PERMIT;
    
    // No right to alter the classify, exit.
    if ((classifyOut->rights & FWPS_RIGHT_ACTION_WRITE) != 0)
    {
        Status = FilterCallbackImpl(
            pClassifyValues, pMetadata, pLayerData);
    }

    if (Status == STATUS_ACCESS_DENIED)
        classifyOut->actionType = FWP_ACTION_BLOCK;
    else    
        classifyOut->actionType = FWP_ACTION_PERMIT;

    if (filter->flags & FWPS_FILTER_FLAG_CLEAR_ACTION_RIGHT)
        classifyOut->rights &= ~FWPS_RIGHT_ACTION_WRITE;
}

NTSTATUS WpfAddFilter(
    _In_ const GUID& layerKey, 
    _In_ const GUID& subLayerKey, 
    _In_ const GUID& calloutKey, 
    _Out_ UINT64* filterId)
{
    FWPM_FILTER filter = { 0 };
    FWPM_FILTER_CONDITION condition[1] = { 0 };

    filter.displayData.name = L"FilterCalloutName";
    filter.displayData.description = L"FilterCalloutName";
    filter.layerKey = layerKey;
    filter.subLayerKey = subLayerKey;
    filter.weight.type = FWP_EMPTY;
    filter.numFilterConditions = 1;
    filter.filterCondition = condition;
    filter.action.type = FWP_ACTION_CALLOUT_TERMINATING;
    filter.action.calloutKey = calloutKey;

    condition[0].fieldKey = FWPM_CONDITION_IP_LOCAL_PORT;
    condition[0].matchType = FWP_MATCH_LESS_OR_EQUAL;
    condition[0].conditionValue.type = FWP_UINT16;
    condition[0].conditionValue.uint16 = (UINT16(-1));

    return FwpmFilterAdd(gEngineHandle, &filter, NULL, filterId);
}

NTSTATUS WpfRegisterFilterCallback(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ const GUID& calloutKey, 
    _In_ const GUID& applicableLayer,
    _In_ const CalloutDescription& desc, 
    _Out_ CalloutContext* context)
{
    NTSTATUS Status;

    FWPS_CALLOUT CalloutS = { 0 };
    CalloutS.calloutKey = calloutKey;
    CalloutS.flags = 0;
    CalloutS.classifyFn = FilterCallback;
    CalloutS.notifyFn = NotifyCallback;
    CalloutS.flowDeleteFn = NULL;
    
    FWPM_CALLOUT CalloutM = { 0 };
    CalloutM.flags = 0;
    CalloutM.displayData.name = desc.name;
    CalloutM.displayData.description = desc.description;
    CalloutM.calloutKey = calloutKey;
    CalloutM.applicableLayer = applicableLayer;

    Status = FwpsCalloutRegister(DeviceObject, &CalloutS, &context->regCalloutId);
    if (!NT_SUCCESS(Status))
        goto end;

    Status = FwpmCalloutAdd(gEngineHandle, &CalloutM, NULL, &context->addCalloutId);
    if (!NT_SUCCESS(Status))
        goto end;

    Status = WpfAddFilter(applicableLayer, 
        NETFILTER_SUBLAYER, calloutKey, &context->filterId);

end:

    kprint_st(TRACE_NETFILTER_INFO, Status);
    return Status;
}

NTSTATUS WpfRegisterInboundCallback(PDEVICE_OBJECT DeviceObject, CalloutContext* context)
{
    CalloutDescription desc;
    desc.name = DRIVER_NAME L" IPv4 inbound transport callout";
    desc.description = L"Drops IPv4 incoming packets not matching firewall rules";

    //FWPS_LAYER_INBOUND_TRANSPORT_V4

    return WpfRegisterFilterCallback(DeviceObject, NETFILTER_CALLOUT_INBOUND_TRANSPORT,
        FWPM_LAYER_INBOUND_TRANSPORT_V4, desc, context);
}

NTSTATUS WpfRegisterOutboundCallback(PDEVICE_OBJECT DeviceObject, CalloutContext* context)
{
    CalloutDescription desc;
    desc.name = DRIVER_NAME L" IPv4 outbound transport callout";
    desc.description = L"Drops IPv4 outcoming packets not matching firewall rules";

    return WpfRegisterFilterCallback(DeviceObject, NETFILTER_CALLOUT_OUTBOUND_TRANSPORT,
        FWPM_LAYER_OUTBOUND_TRANSPORT_V4, desc, context);
}

NTSTATUS WpfAddSublayer()
{
    FWPM_SUBLAYER sublayer = { 0 };
    sublayer.displayData.name = DRIVER_NAME L" filtering layer";
    sublayer.displayData.description = L"Simple antivirus firewall";
    sublayer.subLayerKey = NETFILTER_SUBLAYER;
    sublayer.weight = 0;
    sublayer.flags = 0;

    return FwpmSubLayerAdd(gEngineHandle, &sublayer, NULL);
}

NTSTATUS NetFilterInit(PDRIVER_OBJECT DriverObject)
{
    NTSTATUS Status;
    
    bool ok = NetFilterNewInstance();
    if (!ok)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto end;
    }

    Status = IoCreateDevice(DriverObject, 0, NULL, 
        FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &gDeviceObject);

    if (!NT_SUCCESS(Status))
        goto end;

    Status = FwpmEngineOpen(NULL, 
        RPC_C_AUTHN_WINNT, NULL, NULL, &gEngineHandle);

    if (!NT_SUCCESS(Status))
        goto end;

	Status = WpfAddSublayer();
    if (!NT_SUCCESS(Status))
        goto end;

    Status = WpfRegisterInboundCallback(
        DriverObject->DeviceObject, &gCalloutCtx[IDX_INBOUND]);

    if (!NT_SUCCESS(Status))
        goto end;
   
    Status = WpfRegisterOutboundCallback(
        DriverObject->DeviceObject, &gCalloutCtx[IDX_OUTBOUND]);

    if (!NT_SUCCESS(Status))
        goto end;

end:

    if (!NT_SUCCESS(Status))
    {
        if (NetFilterGetInstancePtr() != nullptr)
            NetFilterDeleteInstance();
    }

    kprint_st(TRACE_NETFILTER_INFO, Status);
    return Status;
}

VOID NetFilterExit()
{
	UINT64 filterId;
	UINT32 addCalloutId;
	UINT32 regCalloutId;


    if (gEngineHandle != NULL)
    {
        for (int i = 0; i < FLT_CNT; i++)
        {
            filterId = gCalloutCtx[i].filterId;
            addCalloutId = gCalloutCtx[i].addCalloutId;
            regCalloutId = gCalloutCtx[i].regCalloutId;
                
            if (filterId != 0)
                FwpmFilterDeleteById(gEngineHandle, filterId);

            if (addCalloutId != 0)
                FwpmCalloutDeleteById(gEngineHandle, addCalloutId);

            if (regCalloutId != 0)
                FwpsCalloutUnregisterById(regCalloutId);
        }

        FwpmSubLayerDeleteByKey(gEngineHandle, &NETFILTER_SUBLAYER);
        FwpmEngineClose(gEngineHandle);
    }

    if (gDeviceObject != NULL)
        IoDeleteDevice(gDeviceObject); 

	if (NetFilterGetInstancePtr() != nullptr)
		NetFilterDeleteInstance();

    kprint_st(TRACE_NETFILTER_INFO, STATUS_SUCCESS);
}
