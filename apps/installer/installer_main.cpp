#include <netlib/netlib.h>
#include <settingslib/settingslib.h>
#include <loglib/loglib.h>
#include <winlib/filesys.h>
#include <common/winreg.h>

#include <thread>
#include <iostream>
#include <conio.h>

#include "installer_workers.h"

#pragma warning (disable:4996)

#define LAUNCHER_APP_NAME L"launcher.exe"

using namespace drjuke;

int main() try
{
    //START_DEFAULT_CONSOLE_LOG();

    auto settings_manager = settingslib::Factory::getSettingsManager();
    auto update_checker   = netlib::Factory::getUpdateChecker();

    // Сначала устанавливаем стандартные настройки
    // продукта в реестр
    settings_manager->setDefaultSettings();

    std::cout << "[SUCCESS] Setting default settings" << std::endl;

    // Создаем необходимые директории
    winlib::filesys::createDirectory(settings_manager->getRootDirectory());
    winlib::filesys::createDirectory(settings_manager->getResourcesDirectory());
    winlib::filesys::createDirectory(settings_manager->getBinariesDirectory());
    winlib::filesys::createDirectory(settings_manager->getQuarantineDirectory());
   
    std::cout << "[SUCCESS] Creating necessary directories" << std::endl;


    // Далее получаем актуальное состояние
    // файлов антивируса
    auto files = update_checker->getActualHashes();

    std::cout << "[SUCCESS] Getting metadata of last build" << std::endl;

    std::cout << "[-------] Downloading actual files:\n";

    // Так как это установка, то нам нужно забрать
    // с сервера все, что у него есть
    std::thread thread_progress_bar { ProgressBarThread };
    std::thread thread_downloader   { DownloaderThread, files, settings_manager->getRootDirectory() };

    // Ожидаем завершения скачивания
    thread_progress_bar.join();
    thread_downloader.join();

    std::cout << "[SUCCESS] Downloading actual files\n";

    // Устанавливаем launcher в автозагрузку
    std::wstring launcher_path = 
        (Path(settings_manager->getBinariesDirectory()) / LAUNCHER_APP_NAME).generic_wstring();

    winreg::RegKey key{ HKEY_LOCAL_MACHINE, LR"(SOFTWARE\Microsoft\Windows\CurrentVersion\Run)" };
    key.SetStringValue(LAUNCHER_APP_NAME, launcher_path);
    std::cout << "[SUCCESS] Adding launcher to auto run\n";
    std::cout << "[-------] Press any key to restart PC" << std::endl;
    _getch();

#if 0
    // Запускаем перезагрузку
    std::wstring shutdown_reason{ L"AVshutdown" };
    ::InitiateShutdownW
    (
        nullptr,
        shutdown_reason.data(),
        0,
        SHUTDOWN_RESTART,
        0
    );
#endif
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