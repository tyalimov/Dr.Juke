#include <netlib/netlib.h>
#include <settingslib/settingslib.h>
#include <loglib/loglib.h>

#include <boost/algorithm/string.hpp>

#include <filesystem>
#include <iostream>
#include <conio.h>

#include "updater_utils.h"
#include "updater_workers.h"
#include <thread>

#pragma warning (disable:4996)

using namespace drjuke;

int main() try 
{
    //START_DEFAULT_CONSOLE_LOG();

    auto update_checker   = netlib::Factory::getUpdateChecker();
    auto settings_manager = settingslib::Factory::getSettingsManager();

    auto root_directory = settings_manager->getRootDirectory();
    std::cout << "[SUCCESS] Getting metadata\n";

    // Посчитать свои хеши
    auto local_hashmap  = GetFilesHashMap(root_directory);
    std::cout << "[SUCCESS] Getting current state\n";

    // Запросить серверные хеши
    auto remote_hashmap = update_checker->getActualHashes();
    std::cout << "[SUCCESS] Getting remote state\n";
     
    // Составить список на скачивание
    auto files_to_download = GetFilesToDownload(local_hashmap, remote_hashmap);

    // Скачать список
    std::thread thread_progress_bar { ProgressBarThread };
    std::thread thread_downloader   { DownloaderThread, files_to_download, root_directory };

    // Ожидаем завершения скачивания
    thread_progress_bar.join();
    thread_downloader.join();

    std::cout << "[SUCCESS] Downloading actual files\n";
    std::cout << "Press any key to exit" << std::endl;
    _getch();
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