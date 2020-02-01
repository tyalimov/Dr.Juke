#pragma once

#include <loglib/loglib.h>

#include <windows.h>

static constexpr int kMaxLoadString{ 100 };

namespace drjuke::updater
{
    class MainWindow
    {
    private:

        DECLARE_CLASS_LOGGER();

        static LRESULT CALLBACK wndCallback(HWND, UINT, WPARAM, LPARAM);

        void registerClass();
        void initInstance();
        void loadStrings();
        void loadAccelerators();

    private:

        // ProgressBarWindow
        // CurrentLoad

        wchar_t   m_title[kMaxLoadString]     { 0 };
        wchar_t   m_wnd_class[kMaxLoadString] { 0 };
        int       m_cmd_show;
        HINSTANCE m_instance_handle;
        MSG       m_message{};
        HACCEL    m_accelerators{};
        HWND      m_window_handle;

    public:
        MainWindow(HINSTANCE instance, int cmd_show);
        ~MainWindow();
        void runMainLoop();
    };
}