#include "ps_protect.h"

#define PROCESS_QUERY_LIMITED_INFORMATION 0x1000


//  The following are for setting up callbacks for Process and Thread filtering
PVOID pCBRegistrationHandle = NULL;

OB_CALLBACK_REGISTRATION  CBObRegistration = { 0 };
OB_OPERATION_REGISTRATION CBOperationRegistrations[2] = { { 0 }, { 0 } };
UNICODE_STRING CBAltitude = { 0 };

PProcessAccessMonitor gPsMon = nullptr;
GuardedMutex gPsMonLock;

PProcessAccessMonitor PsProtectGetInstancePtr() 
{
	LockGuard<GuardedMutex> guard(&gPsMonLock);
	return gPsMon;
}

bool PsProtectNewInstance()
{
	LockGuard<GuardedMutex> guard(&gPsMonLock);

	gPsMon = new ProcessAccessMonitor(KPF_BASE_KEY);
	gPsMon->regReadConfiguration();
	return gPsMon  != nullptr;
}

void PsProtectDeleteInstance()
{
	gPsMonLock.acquire();
	delete gPsMon;
	gPsMon = nullptr;
	gPsMonLock.release();
}

OB_PREOP_CALLBACK_STATUS
PsProtectPreOperationCallback(
    _In_ PVOID RegistrationContext,
    _Inout_ POB_PRE_OPERATION_INFORMATION PreInfo
);

NTSTATUS PsProtectInit()
{
	NTSTATUS Status;

	CBOperationRegistrations[0].ObjectType = PsProcessType;
	CBOperationRegistrations[0].Operations |= OB_OPERATION_HANDLE_CREATE;
	CBOperationRegistrations[0].Operations |= OB_OPERATION_HANDLE_DUPLICATE;
	CBOperationRegistrations[0].PreOperation = PsProtectPreOperationCallback;
	CBOperationRegistrations[0].PostOperation = NULL;

	CBOperationRegistrations[1].ObjectType = PsThreadType;
	CBOperationRegistrations[1].Operations |= OB_OPERATION_HANDLE_CREATE;
	CBOperationRegistrations[1].Operations |= OB_OPERATION_HANDLE_DUPLICATE;
	CBOperationRegistrations[1].PreOperation = PsProtectPreOperationCallback;
	CBOperationRegistrations[1].PostOperation = NULL;


	RtlInitUnicodeString(&CBAltitude, L"1000");

	CBObRegistration.Version = OB_FLT_REGISTRATION_VERSION;
	CBObRegistration.OperationRegistrationCount = 2;
	CBObRegistration.Altitude = CBAltitude;
	CBObRegistration.RegistrationContext = NULL;
	CBObRegistration.OperationRegistration = CBOperationRegistrations;

    bool ok = PsProtectNewInstance();
    if (ok)
    {
        Status = ObRegisterCallbacks(
        	&CBObRegistration,
        	&pCBRegistrationHandle       // save the registration handle to remove callbacks later
        );

        Status = STATUS_SUCCESS;
        if (!NT_SUCCESS(Status))
            PsProtectDeleteInstance();
    }
    else
        Status = STATUS_INSUFFICIENT_RESOURCES;


	kprint_st(TRACE_PSPROTECT, Status);
	return Status;
}

VOID PsProtectExit()
{
	if (PsProtectGetInstancePtr())
	{
        ObUnRegisterCallbacks(pCBRegistrationHandle);
		PsProtectDeleteInstance();
	}

	kprint_st(TRACE_PSPROTECT, STATUS_SUCCESS);
}

OB_PREOP_CALLBACK_STATUS
PsProtectPreOperationCallback (
    _In_ PVOID RegistrationContext,
    _Inout_ POB_PRE_OPERATION_INFORMATION PreInfo
)
{
    UNREFERENCED_PARAMETER(RegistrationContext);

    PACCESS_MASK pDesiredAccess;
    ACCESS_MASK RestrictedAccess;
    PID ProcessId = 0;
	PID CurrentProcessId = PsGetCurrentProcessId();

    if (PreInfo->KernelHandle)
        return OB_PREOP_SUCCESS;

    if (PreInfo->ObjectType == *PsProcessType)  
	{
        ProcessId = PsGetProcessId((PEPROCESS)PreInfo->Object);
        RestrictedAccess = (SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION);

		// Ignore process open/duplicate from the protected process itself
        if (PreInfo->Object == PsGetCurrentProcess())
            goto Exit;
    }
    else if (PreInfo->ObjectType == *PsThreadType)  
	{
        ProcessId = PsGetThreadProcessId((PETHREAD)PreInfo->Object);
        RestrictedAccess = (SYNCHRONIZE | THREAD_QUERY_LIMITED_INFORMATION);

        // Ignore requests for threads belonging to the current processes.
        if (ProcessId == CurrentProcessId) 
            goto Exit;
    }
    else   
	{
		kprintf(TRACE_ERROR, "Unexpected PreInfo->ObjectType");
        goto Exit;
    }

    switch (PreInfo->Operation) 
	{
    case OB_OPERATION_HANDLE_CREATE:
        pDesiredAccess = &PreInfo->Parameters->CreateHandleInformation.DesiredAccess;
        break;

    case OB_OPERATION_HANDLE_DUPLICATE:
        pDesiredAccess = &PreInfo->Parameters->DuplicateHandleInformation.DesiredAccess;
        break;

    default:
		kprintf(TRACE_ERROR, "Unexpected PreInfo->Operation");
        goto Exit;
    }

    if (gPsMon != nullptr)
    {
        bool allowed = gPsMon->accessCheck(CurrentProcessId, ProcessId);

		 if (!allowed)
			*pDesiredAccess = RestrictedAccess;
    }

Exit:
    return OB_PREOP_SUCCESS;
}
