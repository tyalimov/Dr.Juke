#include "main_window.h"
#include "framework.h"
#include "updater.h"

#include <winlib/windows_exception.h>

namespace drjuke::updater
{
    IMPLEMENT_CLASS_LOGGER(MainWindow);

    LRESULT MainWindow::wndCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_COMMAND:
            {
                int wm_id = LOWORD(wParam);
                // Parse the menu selections:
                switch (wm_id)
                {
                case IDM_EXIT:  DestroyWindow(hWnd); break;
                default:        return DefWindowProcW(hWnd, message, wParam, lParam);
                }
            }
        break;

        case WM_DESTROY: PostQuitMessage(0); break;
        default:         return DefWindowProcW(hWnd, message, wParam, lParam);
        }

        return 0;
    }

    void MainWindow::registerClass()
    {
        LOG_INFO(__FUNCTIONW__);

        WNDCLASSEXW wcex;

        wcex.cbSize = sizeof(WNDCLASSEX);

        wcex.style          = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc    = wndCallback;
        wcex.cbClsExtra     = 0;
        wcex.cbWndExtra     = 0;
        wcex.hInstance      = m_instance_handle;
        wcex.hIcon          = LoadIconW(m_instance_handle, MAKEINTRESOURCE(IDI_UPDATER));
        wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground  = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_UPDATER);
        wcex.lpszClassName  = m_wnd_class;
        wcex.hIconSm        = LoadIconW(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

        auto status = RegisterClassExW(&wcex);

        if (!status)
            throw winlib::WindowsException("Can't register class");
    }

    void MainWindow::initInstance()
    {
       LOG_INFO(__FUNCTIONW__);

       HWND h_wnd = CreateWindow
       (
           m_wnd_class, 
           m_title, 
           WS_OVERLAPPEDWINDOW,
           900, 
           700, 
           CW_USEDEFAULT, 
           0, 
           nullptr, 
           nullptr, 
           m_instance_handle,
           nullptr
       );

        if (!h_wnd)
            throw winlib::WindowsException("Can't create window");

        ::ShowWindow(h_wnd, m_cmd_show);
        ::UpdateWindow(h_wnd);
    }

    void MainWindow::loadStrings()
    {
        LOG_INFO(__FUNCTIONW__);
        
        int status = LoadStringW(m_instance_handle, IDS_APP_TITLE, m_title, kMaxLoadString);

        if (!status)
            throw winlib::WindowsException("Can't load m_title");
        

        status = LoadStringW(m_instance_handle, IDC_UPDATER, m_wnd_class, kMaxLoadString);

        if (!status)
            throw winlib::WindowsException("Can't load m_wnd_class");
    }

    void MainWindow::loadAccelerators()
    {
        LOG_INFO(__FUNCTIONW__);

        m_accelerators = LoadAcceleratorsW(m_instance_handle, MAKEINTRESOURCE(IDC_UPDATER));

        if (!m_accelerators)
            throw winlib::WindowsException("Can't load m_accelerators");
    }

    MainWindow::MainWindow(HINSTANCE instance, int cmd_show) try
        : m_cmd_show(cmd_show)
        , m_instance_handle(instance)
    {
        loadStrings();
        registerClass();
        initInstance();
        loadAccelerators();
    }
    catch (const std::exception& ex)
    {
        LOG_FATAL(std::string("Errror creating application - ") + ex.what())
    }

    void MainWindow::runMainLoop() try
    {
        LOG_INFO(__FUNCTIONW__);

        while (GetMessageW(&m_message, nullptr, 0, 0))
        {
            if (!TranslateAcceleratorW(m_message.hwnd, m_accelerators, &m_message))
            {
                TranslateMessage(&m_message);
                DispatchMessageW(&m_message);
            }
        }
    }
    catch (const std::exception& /*ex*/)
    {
        LOG_FATAL("Something went wrong in main loop")
    }
}
