#include "hook_handler.h"
#include "util.h"

VOID NTAPI
on_LdrLoadDll(ApiCall* call);

VOID NTAPI
HookHandlerLower(ApiCall* call)
{
	switch (call->getCallId())
	{
	case CallId::ntdll_LdrLoadDll:
		on_LdrLoadDll(call);
		break;
	default:
		break;
	}

}

VOID NTAPI
on_LdrLoadDll(ApiCall* call)
{
	if (call->isPost())
	{
		PUNICODE_STRING DllName = (PUNICODE_STRING)call->getArgument(2);
		PVOID* DllHandle = (PVOID *)call->getArgument(3);

		UNICODE_STRING NeededDll;
		RtlInitUnicodeString(&NeededDll, (PWSTR)L"kernel32");

		BOOLEAN bkernel32 = RtlPrefixUnicodeString(&NeededDll, DllName, TRUE);
		if (bkernel32)
		{

		}


	}

}
