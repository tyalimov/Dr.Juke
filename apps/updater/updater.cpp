#include "framework.h"
#include "updater.h"
#include "main_window.h"

#include <loglib/loglib.h>

int APIENTRY wWinMain(_In_     HINSTANCE     h_hinstance,
                      _In_opt_ HINSTANCE     /*hPrevInstance*/,
                      _In_     LPWSTR        /*lpCmdLine*/,
                      _In_     int           n_cmd_show)
{
    START_REMOTE_CONSOLE_LOG()

    /*auto main_wnd =*/ drjuke::updater::MainWindow(h_hinstance, n_cmd_show).runMainLoop();
}