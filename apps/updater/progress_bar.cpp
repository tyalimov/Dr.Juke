#include "progress_bar.h"

#include <commctrl.h>
#include <winlib/windows_exception.h>


namespace drjuke::updater
{
    IMPLEMENT_CLASS_LOGGER(ProgressBar);

    void ProgressBar::initWindow()
    {
        LOG_INFO(__FUNCTIONW__);

        m_handle_window = CreateWindowExW
        (
            0, 
            PROGRESS_CLASS, 
            static_cast<LPTSTR>(nullptr),              
            WS_CHILD | WS_VISIBLE, 
            m_parent_rectangle.left,        
            m_parent_rectangle.bottom - m_vscroll,             
            m_parent_rectangle.right, 
            m_vscroll,            
            m_handle_parent, 
            static_cast<HMENU>(nullptr),
            m_instance,
            nullptr
        );
    }

    void ProgressBar::setRange(DWORD from, DWORD to)
    {
        SendMessageW
        (
            m_handle_window, 
            PBM_SETRANGE, 
            0, 
            MAKELPARAM(0, m_file_size)
        );
    }

    void ProgressBar::setPos(DWORD pos)
    {
        SendMessageW
        (
            m_handle_window, 
            PBM_SETPOS , 
            pos, // !!! TODO: new position 
            0
        );
    }

    void ProgressBar::initScroll()
    {
        LOG_INFO(__FUNCTIONW__);

        m_vscroll = GetSystemMetrics(SM_CYVSCROLL);

        if (!m_vscroll)
            throw winlib::WindowsException("Can't get scroll");
    }

    void ProgressBar::initClientRectangle()
    {
        LOG_INFO(__FUNCTIONW__);

        auto status = GetClientRect(m_handle_parent, &m_parent_rectangle);

        if (!status)
            throw winlib::WindowsException("Can't get parent rectangle");
    }

    ProgressBar::ProgressBar(DWORD file_size, HWND parent_window) try
        : m_handle_parent(parent_window)
        , m_file_size(file_size)
    {
        LOG_INFO(__FUNCTIONW__);

        initScroll();
        initClientRectangle();
        initWindow();
        setRange(0, file_size);
    }
    catch (const std::exception& ex)
    {
        LOG_FATAL(std::string("Runtime error - ") + ex.what());
        throw;
    }
}
