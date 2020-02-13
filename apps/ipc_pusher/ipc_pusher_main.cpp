#include <iostream>

#include <sstream>
#include <fstream>
#include <filesystem>

#include <Windows.h>

#include <ipclib/ipclib.h>


int main() try
{
    Sleep(500);
    auto client = drjuke::ipclib::Factory::getCommunicator(drjuke::ipclib::DirectionId::kGuiToService);
    auto result = client->connect();

    if (!result)
    {
        std::cout << "error connecting\n";
        return 0;
    }
    drjuke::Json msg{ "hello", "world" };
    drjuke::Json msg1{ "keke", "lele" };

    client->putMessage(msg);
    client->putMessage(msg1);
}
catch (const std::exception& ex)
{
    std::cout << ex.what() << std::endl;
    system("pause");
    return 0;
}