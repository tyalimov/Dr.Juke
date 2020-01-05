#pragma once

// Driver properties
#define DRIVER_NAME             L"RegFltr"
#define DRIVER_NAME_WITH_EXT    L"RegFltr.sys"

#define NT_DEVICE_NAME          L"\\Device\\RegFltr"
#define DOS_DEVICES_LINK_NAME   L"\\DosDevices\\RegFltr"
#define WIN32_DEVICE_NAME       L"\\\\.\\RegFltr"

// Tracing options
#define TRACE_NONE          0x00000000
#define TRACE_INFO          0x00000001
#define TRACE_ERROR			0x00000002
#define TRACE_THREAD        0x00000004
#define TRACE_PREF			0x00000008
#define TRACE_NOTIFIER		0x00000010
#define TRACE_NOTIFY_OBJ	0x00000020
#define TRACE_CHANGES		0x00000040
#define TRACE_LOAD			0x00000080

#define TRACE_ALL (	TRACE_NONE \
	| TRACE_INFO \
	| TRACE_ERROR \
	| TRACE_THREAD \
	| TRACE_PREF \
	| TRACE_NOTIFIER \
	| TRACE_NOTIFY_OBJ \
	| TRACE_CHANGES \
	| TRACE_LOAD)

#define TRACE_FLAGS TRACE_ALL 
