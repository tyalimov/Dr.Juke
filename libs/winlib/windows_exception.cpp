#include "windows_exception.h"
#include "raii.h"
#include "stacktrace_builder.h"
#include "message_formatter.h"

#include <windows.h>
#include <array>

// TODO: Вносить в консольный лог информацию об ошибках в генерации исключения?

namespace drjuke::winlib
{
    WindowsException::WindowsException(const char *message)
        : m_error_message(message)
        , m_error_code(GetLastError())
    {
    }

    WindowsException::WindowsException(const char *message, 
                                       uint32_t error_code)
        : m_error_message(message)
        , m_error_code(error_code)
    {
    }

    const char *WindowsException::what() const noexcept
    {
        m_message = MessageFormatter(m_error_message, m_error_code).getMessage();
        return m_message.c_str();
    }

    std::string WindowsException::dumpStackTrace() const
    {
        m_message = MessageFormatter(StackTraceBuilder().format()).getMessage();
        return m_message;
    }
}
