#pragma once

// Driver properties
#define DRIVER_NAME             L"RegFltr"
#define DRIVER_NAME_WITH_EXT    L"RegFltr.sys"

#define NT_DEVICE_NAME          L"\\Device\\RegFltr"
#define DOS_DEVICES_LINK_NAME   L"\\DosDevices\\RegFltr"
#define WIN32_DEVICE_NAME       L"\\\\.\\RegFltr"

// Tracing options
#define TRACE_INFO          0x00000001
#define TRACE_ERROR			0x00000002
#define TRACE_THREAD        0x00000004
#define TRACE_PREF			0x00000008

#define TRACE_FLAGS (TRACE_INFO | TRACE_ERROR | TRACE_THREAD | TRACE_PREF)
