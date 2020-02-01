#include <netlib/netlib.h>
#include <settingslib/settingslib.h>
#include <common/aliases.h>
#include <loglib/loglib.h>

#include <thread>
#include <filesystem>
#include <windows.h>

#include "worker_threads.h"
#include <iostream>

using namespace drjuke;

int main() try
{
    //START_DEFAULT_CONSOLE_LOG();

    auto settings_manager = settingslib::Factory::getSettingsManager();
    auto update_checker   = netlib::Factory::getUpdateChecker();
    auto loading_progress = std::make_shared<netlib::LoadingProgress>();

    // Сначала устанавливаем стандартные настройки
    // продукта в реестр
    settings_manager->setDefaultSettings();

    // Далее получаем актуальное состояние
    // файлов антивируса
    auto files = update_checker->getActualHashes();

    std::cout << "Downloading files:\n";

    // Так как это установка, то нам нужно забрать
    // с сервера все, что у него есть
    std::thread thread_progress_bar { ProgressBarThread, loading_progress };
    std::thread thread_downloader   { DownloaderThread, files, fs::current_path(), loading_progress };

    // Ожидаем завершения скачивания
    thread_progress_bar.join();
    thread_downloader.join();

    system("pause");
    // TODO: Дописать в settings_manager отдельные функции
    // TODO: IsuserAdmin

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
catch (const std::exception & ex)
{

}