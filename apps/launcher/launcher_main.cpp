#include <netlib/netlib.h>
#include <settingslib/settingslib.h>
#include <loglib/loglib.h>
#include <winlib/utils.h>
#include <common/winreg.h>

#include <iostream>
#include <conio.h>

#define UPDATER_APP_NAME L"updater.exe"
#define GUI_APP_NAME     L"gui.exe"

using namespace drjuke;

int main() try
{
    auto settings_manager = settingslib::Factory::getSettingsManager();
    auto binaries_directory = settings_manager->getBinariesDirectory();

    auto updater_path = (Path(binaries_directory) / UPDATER_APP_NAME).generic_wstring();
    auto gui_path     = (Path(binaries_directory) / GUI_APP_NAME).generic_wstring();

    // Вызвать updater 
    winlib::utils::startProcess(updater_path, UPDATER_APP_NAME);
    
    // Зарегистрировать сервис и драйверы
    // Передать управление на GUI ! здесь надо создать процесс не с консолью

}
catch (const std::exception& ex)
{
    std::cout << "[FAILURE] " << ex.what() << std::endl;
    std::cout << "Press any key to exit" << std::endl;
    _getch();
}
catch (...)
{
    std::cout << "[FAILURE] Unknown error happened"  << std::endl;
    std::cout << "Press any key to exit" << std::endl;
    _getch();
}
