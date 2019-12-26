#include "stacktrace.h"
#include "raii.h"

#include <windows.h>
#include <Dbghelp.h>
#include <Psapi.h>
#include <memory>
#include <sstream>

namespace drjuke::winlib
{
    void StackTraceBuilder::initializeProcess()
    {
        m_process = UniqueSymHandle(GetCurrentProcess());

        // Инициализируем обработчик символов процесса
        auto status = SymInitialize
        (
            m_process.get(), 
            nullptr, 
            true
        );

        if (!status)
            throw std::runtime_error("can't get process handle");
    }

    void StackTraceBuilder::initializeCapturedFrames()
    {
        m_captured_frames_count = CaptureStackBackTrace
        (
            3, 
            kFramesToCapture, 
            m_captured_frames, 
            nullptr
        );
    }

    void StackTraceBuilder::initializeTrace()
    {
        SYMBOL_INFO symbol[kFramesToCapture];

        // Хз,почему надо именно так
        symbol->MaxNameLen   = kMaxModuleNameSize;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

        char module_name[kMaxModuleNameSize]{0};

        for (auto i = 0; i < m_captured_frames_count; i++)
        {
            // Получаем информацию о символах по адресу
            auto status = SymFromAddr
            (
                m_process.get(), 
                reinterpret_cast<DWORD64>(m_captured_frames[i]), 
                nullptr, 
                symbol
            );

            if (!status)
                break;

            // Получаем полный путь к файлу, содержащему указанный модуль
            status = GetModuleFileNameExA
            (
                m_process.get(), 
                reinterpret_cast<HMODULE>(symbol->ModBase), 
                module_name, 
                kMaxModuleNameSize
            );

            if (!status)
                break;

            m_trace.emplace_back(WindowsStackFrame(symbol->Name, 
                                                   module_name, 
                                                   symbol->Address));
        }
    }

    StackTraceBuilder::StackTraceBuilder() try
        : m_captured_frames_count(0)
        , m_captured_frames{nullptr}
    {
        initializeProcess();
        initializeCapturedFrames();
        initializeTrace();
    }
    catch (const std::exception&) // TODO: написать в лог или нет?
    { 
    }

    std::string StackTraceBuilder::format()
    {
        std::stringstream dumped_stack;

        // Вывод информации в следующем виде:
        // ---- <адрес>:<имя>
        for (const auto& stack_frame : m_trace)
        {
            dumped_stack << stack_frame.m_name << std::endl;
        }

        return dumped_stack.str();
    }
}
