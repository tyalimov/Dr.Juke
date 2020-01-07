#include "message_formatter.h"

#include <windows.h>

namespace drjuke::winlib
{
    void MessageFormatter::formatErrorCode()
    {
        LPSTR buffer;

        constexpr auto flags = 
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM     | 
            FORMAT_MESSAGE_IGNORE_INSERTS;

        const auto status = FormatMessageA
        (
            flags,
            nullptr,
            m_error_code,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPSTR>(&buffer),
            0, 
            nullptr
        );

        m_formatted_error_code = status ? 
            buffer : 
            "FAIL TO GENERATE ERROR MESSAGE";
    }

    void MessageFormatter::addLibraryMessage()
    {
        m_formatted_message += "[ERROR] ";
        m_formatted_message += m_library_message;
        m_formatted_message += "\n";
    }

    void MessageFormatter::addFormattedErrorCode()
    {
        m_formatted_message += "[CAUSE] ";
        m_formatted_message += m_formatted_error_code;
        m_formatted_message += "\n";
    }

    void MessageFormatter::addStackTrace()
    {
        m_formatted_message += "[STACKTRACE]\n";
        m_formatted_message += m_stacktrace;
        m_formatted_message += "\n";
    }

    MessageFormatter::MessageFormatter(const std::string& library_message,
                                       uint32_t errror_code)
        : m_library_message(library_message)
        , m_error_code(errror_code)
    {
        formatErrorCode();
        addLibraryMessage();
        addFormattedErrorCode();
    }

    MessageFormatter::MessageFormatter(const std::string &stacktrace)
        : m_stacktrace(stacktrace)
    {
        addStackTrace();
    }

    std::string MessageFormatter::getMessage() const
    {
        return m_formatted_message;
    }
}
