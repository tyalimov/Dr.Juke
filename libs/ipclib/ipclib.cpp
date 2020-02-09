#include "ipclib.h"
#include "communicator.h"
#include <winlib/windows_exception.h>
#include <common/utils.h>

namespace drjuke::ipclib
{
    static const std::wstring g_pipe_service = L"\\\\.\\pipe\\DrJuke-AVService";
    static const std::wstring g_pipe_gui = L"\\\\.\\pipe\\DrJuke-AVGUI";
    static const std::wstring g_pipe_realtime = L"\\\\.\\pipe\\DrJuke-AVRealtime";
    static const std::wstring g_pipe_scan = L"\\\\.\\pipe\\DrJuke-AVScan";

    CommunicatorPtr Factory::getCommunicator(DirectionId id)
    {
        switch (id)
        {
        case DirectionId::kGuiToService:
            return std::make_shared<Communicator>(
                g_pipe_service, g_pipe_gui, RoleId::kRoleClient);

        case DirectionId::kServiceToGui:
            return std::make_shared<Communicator>(
                g_pipe_gui, g_pipe_service, RoleId::kRoleServer);

        case DirectionId::kReatimeToService:
            return std::make_shared<Communicator>(
                g_pipe_service, g_pipe_realtime, RoleId::kRoleClient);

        case DirectionId::kServiceToRealtime:
            return std::make_shared<Communicator>(
                g_pipe_realtime, g_pipe_service, RoleId::kRoleServer);

        case DirectionId::kScanToService:
            return std::make_shared<Communicator>(
                g_pipe_service, g_pipe_scan, RoleId::kRoleClient);

        case DirectionId::kServiceToScan:
            return std::make_shared<Communicator>(
                g_pipe_scan, g_pipe_service, RoleId::kRoleServer);

        default:
			throw winlib::WindowsException("Unreachable");
        }
    }
}