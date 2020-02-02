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
    auto update_checker   = netlib::Factory::getUpdateChecker();
    auto settings_manager = settingslib::Factory::getSettingsManager();

    auto binaries_directory = settings_manager->getBinariesDirectory();

    // Посчитать свои хеши
    auto local_hashmap  = GetFilesHashMap(binaries_directory);
    
    // Запросить серверные хеши
    auto remote_hashmap = update_checker->getActualHashes();
     
    // Составить список на скачивание
    auto files_to_download = GetFilesToDownload(local_hashmap, remote_hashmap);

    // Скачать список
    std::thread thread_progress_bar { ProgressBarThread };
    std::thread thread_downloader   { DownloaderThread, files_to_download, binaries_directory };

    // Ожидаем завершения скачивания
    thread_progress_bar.join();
    thread_downloader.join();

    std::cout << "[SUCCESS] Downloading actual files\n";
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