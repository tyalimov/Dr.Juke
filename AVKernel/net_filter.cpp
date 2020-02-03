//#include <ntddk.h>
#include "net_filter.h"
#include "net_headers.h"

#include <stdlib.h>

#pragma warning(push)
#pragma warning(disable:4201)       // unnamed struct/union
#include <fwpsk.h>
#pragma warning(pop)

#include <initguid.h>

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
        mBuffer = new CHAR[bytesNeeded];
        if (mBuffer != nullptr)
        {
            mContiguousData = (PCHAR)NdisGetDataBuffer(pNetBuffer,
                bytesNeeded, mBuffer, 1, 0);

            if (mBuffer != mContiguousData)
            {
				// Data is contiguous, no allocation needed
                // Or API call failed and buffer no longer required
				delete[] mBuffer;
				mBuffer = nullptr;

                if (!mContiguousData)
                    kprintf(TRACE_ERROR, "NdisGetDataBuffer failed");
                else
                    mLength = bytesNeeded;
            }
        }
        else
			kprintf(TRACE_ERROR, "Memory allocation failed");
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


    PVOID getData()
    {
        return mBuffer != nullptr
            ? mBuffer
            : mContiguousData;
    }

    ULONG getLength() {
        return mLength;
    }
};

class NetBufferShifter
{
    PNET_BUFFER mNetBuffer = nullptr;
    ULONG mRetreatedCnt = 0;

public:

    NetBufferShifter(PNET_BUFFER pNetBuffer) 
        : mNetBuffer(pNetBuffer) {}

    ~NetBufferShifter()
    {
        restoreDataStart();
        mNetBuffer = nullptr;
        // mRetreatedCnt must be 0
    }

    NTSTATUS retreatDataStart(ULONG nBytesToRetreat)
    {
        NTSTATUS status;

        status = NdisRetreatNetBufferDataStart(
            mNetBuffer, nBytesToRetreat, 0, 0);

        if (NT_SUCCESS(status))
            mRetreatedCnt += nBytesToRetreat;

        return status;
    }

    void advanceDataStart(ULONG nBytesToAdvance)
    {
        mRetreatedCnt -= nBytesToAdvance;

        NdisAdvanceNetBufferDataStart(
            mNetBuffer, nBytesToAdvance, FALSE, 0);
    }

    void restoreDataStart() 
    {
        advanceDataStart(mRetreatedCnt);
        NT_ASSERT(mRetreatedCnt == 0);
    }
};

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
FilterCallback(
    _In_ const FWPS_INCOMING_VALUES* inFixedValues,
    _In_ const FWPS_INCOMING_METADATA_VALUES* inMetaValues,
    _Inout_opt_ void* layerData,
    _In_ const FWPS_FILTER* filter,
    _In_ UINT64 flowContext,
    _Inout_ FWPS_CLASSIFY_OUT* classifyOut
)

#endif /// (NTDDI_VERSION >= NTDDI_WIN7)

{
    UNREFERENCED_PARAMETER(flowContext);
    UNREFERENCED_PARAMETER(classifyContext);

    NT_ASSERT(pLayerData);

    PNET_BUFFER pNetBuffer = NET_BUFFER_LIST_FIRST_NB(
        (NET_BUFFER_LIST*)pLayerData);

	NTSTATUS status = STATUS_SUCCESS;
	FWP_VALUE* pSrcAddrValue = nullptr;
	FWP_VALUE* pDstAddrValue = nullptr;
    FWP_VALUE* pSrcPortValue = nullptr;
	FWP_VALUE* pDstPortValue = nullptr;
	UINT8 protocol = IPPROTO_RAW;
	ULONG ipHeaderSize = 0;


    if (pClassifyValues->layerId != FWPS_LAYER_INBOUND_TRANSPORT_V4)
        return;

    // No right to alter the classify, exit.
    if ((classifyOut->rights & FWPS_RIGHT_ACTION_WRITE) == 0)
        return;

    pSrcAddrValue = &(pClassifyValues->incomingValue[FWPS_FIELD_INBOUND_IPPACKET_V4_IP_REMOTE_ADDRESS].value);
    pDstAddrValue = &(pClassifyValues->incomingValue[FWPS_FIELD_INBOUND_IPPACKET_V4_IP_LOCAL_ADDRESS].value);

    if (!pSrcAddrValue)
    {
        kprintf(TRACE_ERROR, "pSrcAddrValue is NULL");
        return;
    }

    if (!pDstAddrValue)
    {
        kprintf(TRACE_ERROR, "pDstAddrValue is NULL");
        return;
    }

    if (FWPS_IS_METADATA_FIELD_PRESENT(pMetadata, FWPS_METADATA_FIELD_IP_HEADER_SIZE))
        ipHeaderSize = pMetadata->ipHeaderSize;
    else
    {
        kprintf(TRACE_ERROR, "No header size in metadata");
        //return;
    }

    NetBufferShifter shifter(pNetBuffer);
    status = shifter.retreatDataStart(ipHeaderSize);
    if (!NT_SUCCESS(status))
    {
        kprintf(TRACE_ERROR, "NetBufferShifter reatreat failed");
        return;
    }

    if (true)
    {
        kprintf(TRACE_INFO, "Return here");
        return;
    }


    ipHeaderSize = NET_BUFFER_DATA_LENGTH(pNetBuffer);
    NetPacketBuffer ipHeader(pNetBuffer, ipHeaderSize);
    if (!ipHeader.getData())
    {
        kprintf(TRACE_ERROR, "NetPacketBuffer create failed");
        return;
    }

    IP_HEADER_V4* pIPv4Header = (IP_HEADER_V4*)ipHeader.getData();
    if (pIPv4Header->version == IPV4)
    {
        kprintf(TRACE_ERROR, "Bad version");
        return;
    }


    NT_ASSERT(((UINT32)(pIPv4Header->headerLength * 4)) == ipHeaderSize);
    NT_ASSERT(ntohl(*((UINT32*)pIPv4Header->pSourceAddress)) == pSrcAddrValue->uint32);
    NT_ASSERT(ntohl(*((UINT32*)pIPv4Header->pDestinationAddress)) == pDstAddrValue->uint32);

    //LogIPv4Header(pIPv4Header);

    protocol = pIPv4Header->protocol;

    kprintf(TRACE_INFO, "Here");
    RtlZeroMemory(classifyOut, sizeof(FWPS_CLASSIFY_OUT));

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

    kprint_st(TRACE_INFO, Status);
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

    kprint_st(TRACE_INFO, Status);
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
}
