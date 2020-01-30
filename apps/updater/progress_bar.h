#pragma once

#include <loglib/loglib.h>
#include <windows.h>

namespace drjuke::updater
{
    class ProgressBar
    {
    private:
        DECLARE_CLASS_LOGGER();
        
        HWND      m_handle_window;
        HWND      m_handle_parent;
        DWORD     m_file_size;
        RECT      m_parent_rectangle;
        HINSTANCE m_instance;
        int       m_vscroll;
    
    private:
        void initScroll();
        void initClientRectangle();
        void initWindow();


    public:
        void setRange(DWORD from, DWORD to);
        void setPos(DWORD pos);
        
        ProgressBar(DWORD file_size, HWND parent_window);
    };
}