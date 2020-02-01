#pragma once

#include <loglib/loglib.h>
#include <windows.h>

#include <netlib/netlib.h>

namespace drjuke::updater
{
    class ProgressBarWindow
    {
    private:
        DECLARE_CLASS_LOGGER();
        
        HWND      m_handle_window;
        HWND      m_handle_parent;
        RECT      m_parent_rectangle;
        HINSTANCE m_instance;
        int       m_vscroll;
        
        DWORD     m_file_size;

        netlib::LoadingProgressPtr m_loading_progress;
    
    private:
        void initScroll();
        void initClientRectangle();
        void initWindow();


    public:
        void setRange(DWORD from, DWORD to);
        void setPos(DWORD pos);
        
        ProgressBarWindow(HWND parent_window, netlib::LoadingProgressPtr progress);
    };
}