#include "except.h"
#include "raii.h"

#include <windows.h>
#include <Dbghelp.h>
#include <Psapi.h>
#include <memory>
#include <sstream>
#include <array>

namespace drjuke::winlib
{
    void WindowsException::initializeMessage(const char *message)
    {
        LPSTR buffer;

        FormatMessageA
        (
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            m_error_code,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPSTR>(&buffer),
            0, 
            nullptr
        );
    }

    void WindowsException::initializeStackTrace()
    {
        constexpr uint32_t frames_to_capture     = 64;
        constexpr uint32_t max_module_name_size  = 512;

        UniqueSymHandle process(GetCurrentProcess());

        // Инициализируем обработчик символов процесса
        auto status = SymInitialize
        (
            process.get(), 
            nullptr, 
            true
        );

        if (!status)
            return;

        void *captured_frames[frames_to_capture];

        WORD captured_frames_count = CaptureStackBackTrace
        (
            1, 
            frames_to_capture, 
            captured_frames, 
            nullptr
        );

        std::unique_ptr<SYMBOL_INFO> symbol(new SYMBOL_INFO[frames_to_capture]);

        symbol->MaxNameLen   = max_module_name_size;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

        char module_name[max_module_name_size]{0};

        for(unsigned int i = 0; i < captured_frames_count; i++)
        {
            // Получаем информацию о символах по адресу
            status = SymFromAddr
            (
                process.get(), 
                reinterpret_cast<DWORD64>(captured_frames[i]), 
                nullptr, 
                symbol.get()
            );

            if (!status)
                break;

            // Получаем полный путь к файлу, содержащему указанный модуль
            status = GetModuleFileNameExA
            (
                process.get(), 
                reinterpret_cast<HMODULE>(symbol->ModBase), 
                module_name, 
                max_module_name_size
            );

            if (!status)
                break;

            m_stack_trace.emplace_back
            (
                WindowsStackFrame(symbol->Name, 
                                  module_name, 
                                  symbol->Address)
            );
        }
    }

    WindowsException::WindowsException(const char *message)
        : m_stack_trace()
        , m_error_code(GetLastError())
        , m_error_message()

    {
        initializeMessage(message);
        initializeStackTrace();
    }

    const char *WindowsException::what() const noexcept
    {
        return m_error_message.c_str();
    }

    std::string WindowsException::dumpStackTrace() const
    {
        std::stringstream dumped_stack;

        // Вывести причину
        dumped_stack << what() << std::endl;

        // Вывод информации в следующем виде:
        // ---- <адрес>:<модуль>:<имя>
        for(const auto& stack_frame : m_stack_trace)
        {
            dumped_stack << "---- " << "0x" << std::hex << stack_frame.m_address
                << ":" << stack_frame.m_module << ":" << stack_frame.m_name << std::endl;
        }

        return dumped_stack.str();
    }
}
